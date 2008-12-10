import FWCore.ParameterSet.Config as cms

# producer of rechits starting from digis
ecalWeightUncalibRecHit = cms.EDProducer("EcalWeightUncalibRecHitProducer",
    EBdigiCollection = cms.InputTag("ecalDigis","ebDigis"),
    EEhitCollection = cms.string('EcalUncalibRecHitsEE'),
    EEdigiCollection = cms.InputTag("ecalDigis","eeDigis"),
    EBhitCollection = cms.string('EcalUncalibRecHitsEB')
)

#ecalWeightUncalibRecHit = cms.EDProducer("EcalUncalibRecHitProducer",
#                                         algo = cms.string('EcalUncalibRecHitRecWeightsAlgoWrapper'),
#                                         EBdigiCollection = cms.InputTag("ecalDigis","ebDigis"),
#                                         EEhitCollection = cms.string('EcalUncalibRecHitsEE'),
#                                         EEdigiCollection = cms.InputTag("ecalDigis","eeDigis"),
#                                         EBhitCollection = cms.string('EcalUncalibRecHitsEB')
#                                         )

