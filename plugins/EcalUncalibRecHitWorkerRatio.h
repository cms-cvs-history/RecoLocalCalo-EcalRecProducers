#ifndef RecoLocalCalo_EcalRecProducers_EcalUncalibRecHitRecWorkerRatio_hh
#define RecoLocalCalo_EcalRecProducers_EcalUncalibRecHitRecWorkerRatio_hh

/** \class EcalUncalibRecHitRecRatioAlgo
  *  Template used to compute amplitude, pedestal, time jitter, chi2 of a pulse
  *  using a weights method
  *
  *  $Id: EcalUncalibRecHitWorkerRatio.h,v 1.1 2009/02/19 22:24:43 franzoni Exp $
  *  $Date: 2009/02/19 22:24:43 $
  *  $Revision: 1.1 $
  *  \author A. Ledovskoy (Design) - M. Balazs (Implementation)
  */

#include "RecoLocalCalo/EcalRecProducers/interface/EcalUncalibRecHitWorkerBaseClass.h"
#include "RecoLocalCalo/EcalRecAlgos/interface//EcalUncalibRecHitRatioMethodAlgo.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/EcalObjects/interface/EcalGainRatios.h"

namespace edm {
        class Event;
        class EventSetup;
        class ParameterSet;
}

class EcalUncalibRecHitWorkerRatio : public EcalUncalibRecHitWorkerBaseClass {

        public:
                EcalUncalibRecHitWorkerRatio(const edm::ParameterSet&);
                virtual ~EcalUncalibRecHitWorkerRatio() {};

                void set(const edm::EventSetup& es);
                bool run(const edm::Event& evt, const EcalDigiCollection::const_iterator & digi, EcalUncalibratedRecHitCollection & result);

        protected:

                edm::ESHandle<EcalPedestals> peds;
                edm::ESHandle<EcalGainRatios>  gains;

                double pedVec[3];
                double pedRMSVec[3];
                double gainRatios[3];

		std::vector<double> EBtimeFitParameters_; 
		std::vector<double> EEtimeFitParameters_; 
 
		std::vector<double> EBamplitudeFitParameters_; 
		std::vector<double> EEamplitudeFitParameters_; 
 
		std::pair<double,double> EBtimeFitLimits_;  
		std::pair<double,double> EEtimeFitLimits_;  
                
                EcalUncalibRecHitRatioMethodAlgo<EBDataFrame> uncalibMaker_barrel_;
                EcalUncalibRecHitRatioMethodAlgo<EEDataFrame> uncalibMaker_endcap_;
};

#endif
