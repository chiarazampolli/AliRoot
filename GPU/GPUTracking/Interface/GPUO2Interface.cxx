//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file GPUO2Interface.cxx
/// \author David Rohr

#include "GPUO2Interface.h"
#include "GPUReconstruction.h"
#include "GPUChainTracking.h"
#include "GPUMemorySizeScalers.h"
#include "GPUOutputControl.h"
#include "GPUO2InterfaceConfiguration.h"
#include "GPUParam.inc"
#include "GPUQA.h"
#include <iostream>
#include <fstream>
#ifdef WITH_OPENMP
#include <omp.h>
#endif

using namespace o2::gpu;

#include "DataFormatsTPC/ClusterNative.h"

GPUTPCO2Interface::GPUTPCO2Interface() = default;

GPUTPCO2Interface::~GPUTPCO2Interface() { Deinitialize(); }

int GPUTPCO2Interface::Initialize(const GPUO2InterfaceConfiguration& config)
{
  if (mInitialized) {
    return (1);
  }
  mConfig.reset(new GPUO2InterfaceConfiguration(config));
  mContinuous = mConfig->configEvent.continuousMaxTimeBin != 0;
  mRec.reset(GPUReconstruction::CreateInstance(mConfig->configDeviceBackend));
  if (mRec == nullptr) {
    GPUError("Error obtaining instance of GPUReconstruction");
    return 1;
  }
  mChain = mRec->AddChain<GPUChainTracking>(mConfig->configInterface.maxTPCHits, mConfig->configInterface.maxTRDTracklets);
  mChain->mConfigDisplay = &mConfig->configDisplay;
  mChain->mConfigQA = &mConfig->configQA;
  if (mConfig->configWorkflow.inputs.isSet(GPUDataTypes::InOutType::TPCRaw)) {
    mConfig->configEvent.needsClusterer = 1;
  }
  mRec->SetSettings(&mConfig->configEvent, &mConfig->configReconstruction, &mConfig->configProcessing, &mConfig->configWorkflow);
  mChain->SetTPCFastTransform(mConfig->configCalib.fastTransform);
  mChain->SetTPCCFCalibration(mConfig->configCalib.tpcCalibration);
  mChain->SetdEdxSplines(mConfig->configCalib.dEdxSplines);
  mChain->SetMatLUT(mConfig->configCalib.matLUT);
  mChain->SetTRDGeometry(mConfig->configCalib.trdGeometry);
  if (mConfig->configInterface.outputToExternalBuffers) {
    mOutputCompressedClusters.reset(new GPUOutputControl);
    mChain->SetOutputControlCompressedClusters(mOutputCompressedClusters.get());
    mOutputClustersNative.reset(new GPUOutputControl);
    mChain->SetOutputControlClustersNative(mOutputClustersNative.get());
    mOutputTPCTracks.reset(new GPUOutputControl);
    mChain->SetOutputControlTPCTracks(mOutputTPCTracks.get());
    GPUOutputControl dummy;
    dummy.set([](size_t size) -> void* {throw std::runtime_error("invalid output memory request, no common output buffer set"); return nullptr; });
    mRec->SetOutputControl(dummy);
  }
  if (mConfig->configProcessing.runMC) {
    mOutputTPCClusterLabels.reset(new GPUOutputControl);
    mChain->SetOutputControlClusterLabels(mOutputTPCClusterLabels.get());
  }

  if (mRec->Init()) {
    return (1);
  }
  if (!mRec->IsGPU() && mConfig->configProcessing.memoryAllocationStrategy == GPUMemoryResource::ALLOCATION_INDIVIDUAL) {
    mRec->MemoryScalers()->factor *= 2;
  }
  mRec->MemoryScalers()->factor *= mConfig->configInterface.memoryBufferScaleFactor;
  mInitialized = true;
  return (0);
}

void GPUTPCO2Interface::Deinitialize()
{
  if (mInitialized) {
    mRec->Finalize();
    mRec.reset();
  }
  mInitialized = false;
}

int GPUTPCO2Interface::RunTracking(GPUTrackingInOutPointers* data, GPUInterfaceOutputs* outputs)
{
  if (!mInitialized) {
    return (1);
  }
  static int nEvent = 0;
  if (mConfig->configInterface.dumpEvents) {
    mChain->ClearIOPointers();
    mChain->mIOPtrs.clustersNative = data->clustersNative;
    mChain->mIOPtrs.tpcPackedDigits = data->tpcPackedDigits;
    mChain->mIOPtrs.tpcZS = data->tpcZS;

    char fname[1024];
    sprintf(fname, "event.%d.dump", nEvent);
    mChain->DumpData(fname);
    if (nEvent == 0) {
      mRec->DumpSettings();
    }
    if (mConfig->configInterface.dumpEvents >= 2) {
      return 0;
    }
  }

  mChain->mIOPtrs = *data;
  if (mConfig->configInterface.outputToExternalBuffers) {
    if (outputs->compressedClusters.allocator) {
      mOutputCompressedClusters->set(outputs->compressedClusters.allocator);
    } else if (outputs->compressedClusters.ptr) {
      mOutputCompressedClusters->set(outputs->compressedClusters.ptr, outputs->compressedClusters.size);
    } else {
      mOutputCompressedClusters->reset();
    }
    if (outputs->clustersNative.allocator) {
      mOutputClustersNative->set(outputs->clustersNative.allocator);
    } else if (outputs->clustersNative.ptr) {
      mOutputClustersNative->set(outputs->clustersNative.ptr, outputs->clustersNative.size);
    } else {
      mOutputClustersNative->reset();
    }
    if (outputs->tpcTracks.allocator) {
      mOutputTPCTracks->set(outputs->tpcTracks.allocator);
    } else if (outputs->tpcTracks.ptr) {
      mOutputTPCTracks->set(outputs->tpcTracks.ptr, outputs->tpcTracks.size);
    } else {
      mOutputTPCTracks->reset();
    }
  }
  if (mConfig->configProcessing.runMC) {
    if (outputs->clusterLabels.allocator) {
      mOutputTPCClusterLabels->set(outputs->clusterLabels.allocator);
    } else {
      mOutputTPCClusterLabels->reset();
    }
  }
  int retVal = mRec->RunChains();
  if (retVal == 2) {
    retVal = 0; // 2 signals end of event display, ignore
  }
  if (retVal) {
    mRec->ClearAllocatedMemory();
    return retVal;
  }
  if (mConfig->configInterface.outputToExternalBuffers) {
    outputs->compressedClusters.size = mOutputCompressedClusters->EndOfSpace ? 0 : mChain->mIOPtrs.tpcCompressedClusters->totalDataSize;
    outputs->clustersNative.size = mOutputClustersNative->EndOfSpace ? 0 : (mChain->mIOPtrs.clustersNative->nClustersTotal * sizeof(*mChain->mIOPtrs.clustersNative->clustersLinear));
    outputs->tpcTracks.size = mOutputCompressedClusters->EndOfSpace ? 0 : (size_t)((char*)mOutputCompressedClusters->OutputPtr - (char*)mOutputCompressedClusters->OutputBase);
  }
  if (mConfig->configQA.shipToQC) {
    outputs->qa.hist1 = &mChain->GetQA()->getHistograms1D();
    outputs->qa.hist2 = &mChain->GetQA()->getHistograms2D();
    outputs->qa.hist3 = &mChain->GetQA()->getHistograms1Dd();
  }
  *data = mChain->mIOPtrs;

  nEvent++;
  return 0;
}

void GPUTPCO2Interface::Clear(bool clearOutputs) { mRec->ClearAllocatedMemory(clearOutputs); }

void GPUTPCO2Interface::GetClusterErrors2(int row, float z, float sinPhi, float DzDs, short clusterState, float& ErrY2, float& ErrZ2) const
{
  mRec->GetParam().GetClusterErrors2(row, z, sinPhi, DzDs, ErrY2, ErrZ2);
  mRec->GetParam().UpdateClusterError2ByState(clusterState, ErrY2, ErrZ2);
}

int GPUTPCO2Interface::registerMemoryForGPU(const void* ptr, size_t size)
{
  return mRec->registerMemoryForGPU(ptr, size);
}

int GPUTPCO2Interface::unregisterMemoryForGPU(const void* ptr)
{
  return mRec->unregisterMemoryForGPU(ptr);
}
