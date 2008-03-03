/** \class EcalRecalibRecHitProducer
 *   produce ECAL rechits from uncalibrated rechits
 *
 *  $Id: EcalRecalibRecHitProducer.cc,v 1.14 2007/09/27 10:14:21 ferriff Exp $
 *  $Date: 2007/09/27 10:14:21 $
 *  $Revision: 1.14 $
 *  \author Shahram Rahatlou, University of Rome & INFN, March 2006
 *
 **/
#include "RecoLocalCalo/EcalRecProducers/interface/EcalRecalibRecHitProducer.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalRecHitSimpleAlgo.h"
#include "CondFormats/EcalObjects/interface/EcalIntercalibConstants.h"
#include "CondFormats/DataRecord/interface/EcalIntercalibConstantsRcd.h"
#include "CondFormats/EcalObjects/interface/EcalADCToGeVConstant.h"
#include "CondFormats/DataRecord/interface/EcalADCToGeVConstantRcd.h"
#include "CalibCalorimetry/EcalLaserCorrection/interface/EcalLaserDbService.h"
#include "CalibCalorimetry/EcalLaserCorrection/interface/EcalLaserDbRecord.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <iostream>
#include <cmath>
#include <vector>

//#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
//#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"
#include "DataFormats/EcalRecHit/interface/EcalUncalibratedRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"



EcalRecalibRecHitProducer::EcalRecalibRecHitProducer(const edm::ParameterSet& ps) {

   EBRecHitCollection_ = ps.getParameter<edm::InputTag>("EBRecHitCollection");
   EERecHitCollection_ = ps.getParameter<edm::InputTag>("EERecHitCollection");
   EBRecalibRecHitCollection_        = ps.getParameter<std::string>("EBRecalibRechitCollection");
   EERecalibRecHitCollection_        = ps.getParameter<std::string>("EERecalibRechitCollection");
   doEnergyScale_             = ps.getParameter<bool>("doEnergyScale");
   doIntercalib_              = ps.getParameter<bool>("doIntercalib");
   doLaserCorrections_        = ps.getParameter<bool>("doLaserCorrections");

   EBalgo_ = new EcalRecHitSimpleAlgo();
   EEalgo_ = new EcalRecHitSimpleAlgo();

   produces< EBRecHitCollection >(EBRecalibRecHitCollection_);
   produces< EERecHitCollection >(EERecalibRecHitCollection_);
}

EcalRecalibRecHitProducer::~EcalRecalibRecHitProducer() {

  if (EBalgo_) delete EBalgo_;
  if (EEalgo_) delete EEalgo_;

}

void EcalRecalibRecHitProducer::produce(edm::Event& evt, const edm::EventSetup& es)
{
        using namespace edm;
        Handle< EBRecHitCollection > pEBRecHits;
        Handle< EERecHitCollection > pEERecHits;

        const EBRecHitCollection*  EBRecHits = 0;
        const EERecHitCollection*  EERecHits = 0; 

        try {
                evt.getByLabel( EBRecHitCollection_, pEBRecHits);
                EBRecHits = pEBRecHits.product(); // get a ptr to the product
#ifdef DEBUG
                LogDebug("EcalRecHitDebug") << "total # EB rechits to be re-calibrated: " << EBuncalibRecHits->size();
#endif
        } catch (...) {
                //edm::LogError("EcalRecHitError") << "Error! can't get the product " << EBuncalibRecHitCollection_.c_str() ;
        }

        try {
                evt.getByLabel( EERecHitCollection_, pEERecHits);
                EERecHits = pEERecHits.product(); // get a ptr to the product
#ifdef DEBUG
                LogDebug("EcalRecHitDebug") << "total # EE uncalibrated rechits to be re-calibrated: " << EEuncalibRecHits->size();
#endif
        } catch (...) {
                //edm::LogError("EcalRecHitError") << "Error! can't get the product " << EEuncalibRecHitCollection_.c_str() ;
        }

        // collection of rechits to put in the event
        std::auto_ptr< EBRecHitCollection > EBRecalibRecHits( new EBRecHitCollection );
        std::auto_ptr< EERecHitCollection > EERecalibRecHits( new EERecHitCollection );

        // now fetch all conditions we need to make rechits
        // ADC to GeV constant
        edm::ESHandle<EcalADCToGeVConstant> pAgc;
        const EcalADCToGeVConstant *agc = 0;
        float agc_eb = 1.;
        float agc_ee = 1.;
        if (doEnergyScale_) {
                es.get<EcalADCToGeVConstantRcd>().get(pAgc);
                agc = pAgc.product();
                // use this value in the algorithm
                agc_eb = float(agc->getEBValue());
                agc_ee = float(agc->getEEValue());
        }
        // Intercalib constants
        edm::ESHandle<EcalIntercalibConstants> pIcal;
        const EcalIntercalibConstants *ical = 0;
        if (doIntercalib_) {
                es.get<EcalIntercalibConstantsRcd>().get(pIcal);
                ical = pIcal.product();
        }
        // Laser corrections
        edm::ESHandle<EcalLaserDbService> pLaser;
        es.get<EcalLaserDbRecord>().get( pLaser );

        if (EBRecHits) {
                // loop over uncalibrated rechits to make calibrated ones
                for(EBRecHitCollection::const_iterator it  = EBRecHits->begin(); it != EBRecHits->end(); ++it) {

                        EcalIntercalibConstant icalconst = 1.;
                        if (doIntercalib_) {
                                // find intercalib constant for this xtal
                                const EcalIntercalibConstantMap &icalMap = ical->getMap();
                                EcalIntercalibConstantMap::const_iterator icalit = icalMap.find(it->id());
                                if( icalit!=icalMap.end() ){
                                        icalconst = (*icalit);
                                } else {
                                        edm::LogError("EcalRecHitError") << "No intercalib const found for xtal " << EBDetId(it->id()) << "! something wrong with EcalIntercalibConstants in your DB? "
                                                ;
                                }
                        }
                        // get laser coefficient
                        float lasercalib = 1;
                        if (doLaserCorrections_) {
                                pLaser->getLaserCorrection( EBDetId(it->id()), evt.time() );
                        }

                        // make the rechit and put in the output collection
                        // must implement op= for EcalRecHit
                        EcalRecHit aHit( (*it).id(), (*it).energy() * agc_eb * icalconst * lasercalib, (*it).time() );
                        EBRecalibRecHits->push_back( aHit );
                }
        }

        if (EERecHits)
        {
                // loop over uncalibrated rechits to make calibrated ones
                for(EERecHitCollection::const_iterator it  = EERecHits->begin();
                                it != EERecHits->end(); ++it) {

                        // find intercalib constant for this xtal
                        EcalIntercalibConstant icalconst = 1.;
                        if (doIntercalib_) {
                                const EcalIntercalibConstantMap &icalMap = ical->getMap();
                                EcalIntercalibConstantMap::const_iterator icalit=icalMap.find(it->id());
                                if( icalit!=icalMap.end() ) {
                                        icalconst = (*icalit);
                                } else {
                                        edm::LogError("EcalRecHitError") << "No intercalib const found for xtal " << EEDetId(it->id()) << "! something wrong with EcalIntercalibConstants in your DB? ";
                                }
                        }
                        // get laser coefficient
                        float lasercalib = 1;
                        if (doLaserCorrections_) {
                                pLaser->getLaserCorrection( EEDetId(it->id()), evt.time() );
                        }

                        // make the rechit and put in the output collection
                        // must implement op= for EcalRecHit
                        //EcalRecHit aHit( EEalgo_->makeRecHit(*it, icalconst * lasercalib) );
                        EcalRecHit aHit( (*it).id(), (*it).energy() * agc_ee * icalconst * lasercalib, (*it).time() );
                        EERecalibRecHits->push_back( aHit );
                }
        }
        // put the collection of recunstructed hits in the event   
        LogInfo("EcalRecalibRecHitInfo") << "total # EB re-calibrated rechits: " << EBRecHits->size();
        LogInfo("EcalRecalibRecHitInfo") << "total # EE re-calibrated rechits: " << EERecHits->size();

        evt.put( EBRecalibRecHits, EBRecalibRecHitCollection_ );
        evt.put( EERecalibRecHits, EERecalibRecHitCollection_ );
}