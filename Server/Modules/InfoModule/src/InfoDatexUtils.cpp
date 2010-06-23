/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "InfoDatexUtils.h"

map<TrafficDataTypes::phrase, 
    StringTableUTF8::stringCode> InfoDatexUtils::c_phraseStringMap;

bool InfoDatexUtils::initializeTables()
{
   {
      typedef std::pair<TrafficDataTypes::phrase, 
         StringTableUTF8::stringCode> pp;
      const pp phraseString_map[] = { 
#if 1
    pp(TrafficDataTypes::ABL, StringTableUTF8::TRAFFIC_ABL),
    pp(TrafficDataTypes::ABX, StringTableUTF8::TRAFFIC_ABX),
    pp(TrafficDataTypes::ACA, StringTableUTF8::TRAFFIC_ACA),
    pp(TrafficDataTypes::ACB, StringTableUTF8::TRAFFIC_ACB),
    pp(TrafficDataTypes::ACD, StringTableUTF8::TRAFFIC_ACD),
    pp(TrafficDataTypes::ACE, StringTableUTF8::TRAFFIC_ACE),
    pp(TrafficDataTypes::ACF, StringTableUTF8::TRAFFIC_ACF),
    pp(TrafficDataTypes::ACH, StringTableUTF8::TRAFFIC_ACH),
    pp(TrafficDataTypes::ACI, StringTableUTF8::TRAFFIC_ACI),
    pp(TrafficDataTypes::ACL, StringTableUTF8::TRAFFIC_ACL),
    pp(TrafficDataTypes::ACM, StringTableUTF8::TRAFFIC_ACM),
    pp(TrafficDataTypes::ACS, StringTableUTF8::TRAFFIC_ACS),
    pp(TrafficDataTypes::ACW, StringTableUTF8::TRAFFIC_ACW),
    pp(TrafficDataTypes::ACX, StringTableUTF8::TRAFFIC_ACX),
    pp(TrafficDataTypes::ACZ, StringTableUTF8::TRAFFIC_ACZ),
    pp(TrafficDataTypes::AIC, StringTableUTF8::TRAFFIC_AIC),
    pp(TrafficDataTypes::AIR, StringTableUTF8::TRAFFIC_AIR),
    pp(TrafficDataTypes::AJA, StringTableUTF8::TRAFFIC_AJA),
    pp(TrafficDataTypes::AJC, StringTableUTF8::TRAFFIC_AJC),
    pp(TrafficDataTypes::AJT, StringTableUTF8::TRAFFIC_AJT),
    pp(TrafficDataTypes::ALA, StringTableUTF8::TRAFFIC_ALA),
    pp(TrafficDataTypes::ALC, StringTableUTF8::TRAFFIC_ALC),
    pp(TrafficDataTypes::ALL, StringTableUTF8::TRAFFIC_ALL),
    pp(TrafficDataTypes::ALR, StringTableUTF8::TRAFFIC_ALR),
    pp(TrafficDataTypes::ANH, StringTableUTF8::TRAFFIC_ANH),
    pp(TrafficDataTypes::ANL, StringTableUTF8::TRAFFIC_ANL),
    pp(TrafficDataTypes::ANM, StringTableUTF8::TRAFFIC_ANM),
    pp(TrafficDataTypes::AOI, StringTableUTF8::TRAFFIC_AOI),
    pp(TrafficDataTypes::AOL, StringTableUTF8::TRAFFIC_AOL),
    pp(TrafficDataTypes::AOV, StringTableUTF8::TRAFFIC_AOV),
    pp(TrafficDataTypes::APF, StringTableUTF8::TRAFFIC_APF),
    pp(TrafficDataTypes::APT, StringTableUTF8::TRAFFIC_APT),
    pp(TrafficDataTypes::AQD, StringTableUTF8::TRAFFIC_AQD),
    pp(TrafficDataTypes::ASP, StringTableUTF8::TRAFFIC_ASP),
    pp(TrafficDataTypes::ATR, StringTableUTF8::TRAFFIC_ATR),
    pp(TrafficDataTypes::AVL, StringTableUTF8::TRAFFIC_AVL),
    pp(TrafficDataTypes::AVS, StringTableUTF8::TRAFFIC_AVS),
    pp(TrafficDataTypes::BCL, StringTableUTF8::TRAFFIC_BCL),
    pp(TrafficDataTypes::BDB, StringTableUTF8::TRAFFIC_BDB),
    pp(TrafficDataTypes::BDX, StringTableUTF8::TRAFFIC_BDX),
    pp(TrafficDataTypes::BKD, StringTableUTF8::TRAFFIC_BKD),
    pp(TrafficDataTypes::BLI, StringTableUTF8::TRAFFIC_BLI),
    pp(TrafficDataTypes::BLO, StringTableUTF8::TRAFFIC_BLO),
    pp(TrafficDataTypes::BLS, StringTableUTF8::TRAFFIC_BLS),
    pp(TrafficDataTypes::BRB, StringTableUTF8::TRAFFIC_BRB),
    pp(TrafficDataTypes::BRC, StringTableUTF8::TRAFFIC_BRC),
    pp(TrafficDataTypes::BRP, StringTableUTF8::TRAFFIC_BRP),
    pp(TrafficDataTypes::BSI, StringTableUTF8::TRAFFIC_BSI),
    pp(TrafficDataTypes::BUN, StringTableUTF8::TRAFFIC_BUN),
    pp(TrafficDataTypes::BWM, StringTableUTF8::TRAFFIC_BWM),
    pp(TrafficDataTypes::CAA, StringTableUTF8::TRAFFIC_CAA),
    pp(TrafficDataTypes::CAC, StringTableUTF8::TRAFFIC_CAC),
    pp(TrafficDataTypes::CAL, StringTableUTF8::TRAFFIC_CAL),
    pp(TrafficDataTypes::CBW, StringTableUTF8::TRAFFIC_CBW),
    pp(TrafficDataTypes::CCB, StringTableUTF8::TRAFFIC_CCB),
    pp(TrafficDataTypes::CCC, StringTableUTF8::TRAFFIC_CCC),
    pp(TrafficDataTypes::CHI, StringTableUTF8::TRAFFIC_CHI),
    pp(TrafficDataTypes::CLE, StringTableUTF8::TRAFFIC_CLE),
    pp(TrafficDataTypes::CLW, StringTableUTF8::TRAFFIC_CLW),
    pp(TrafficDataTypes::CON, StringTableUTF8::TRAFFIC_CON),
    pp(TrafficDataTypes::COO, StringTableUTF8::TRAFFIC_COO),
    pp(TrafficDataTypes::COW, StringTableUTF8::TRAFFIC_COW),
    pp(TrafficDataTypes::CPW, StringTableUTF8::TRAFFIC_CPW),
    pp(TrafficDataTypes::CRC, StringTableUTF8::TRAFFIC_CRC),
    pp(TrafficDataTypes::CRX, StringTableUTF8::TRAFFIC_CRX),
    pp(TrafficDataTypes::CTR, StringTableUTF8::TRAFFIC_CTR),
    pp(TrafficDataTypes::CTT, StringTableUTF8::TRAFFIC_CTT),
    pp(TrafficDataTypes::CTX, StringTableUTF8::TRAFFIC_CTX),
    pp(TrafficDataTypes::CV1, StringTableUTF8::TRAFFIC_CV1),
    pp(TrafficDataTypes::CV3, StringTableUTF8::TRAFFIC_CV3),
    pp(TrafficDataTypes::CVX, StringTableUTF8::TRAFFIC_CVX),
    pp(TrafficDataTypes::CVY, StringTableUTF8::TRAFFIC_CVY),
    pp(TrafficDataTypes::CWC, StringTableUTF8::TRAFFIC_CWC),
    pp(TrafficDataTypes::CYC, StringTableUTF8::TRAFFIC_CYC),
    pp(TrafficDataTypes::DCD, StringTableUTF8::TRAFFIC_DCD),
    pp(TrafficDataTypes::DCE, StringTableUTF8::TRAFFIC_DCE),
    pp(TrafficDataTypes::DCL, StringTableUTF8::TRAFFIC_DCL),
    pp(TrafficDataTypes::DCN, StringTableUTF8::TRAFFIC_DCN),
    pp(TrafficDataTypes::DCZ, StringTableUTF8::TRAFFIC_DCZ),
    pp(TrafficDataTypes::DEI, StringTableUTF8::TRAFFIC_DEI),
    pp(TrafficDataTypes::DEU, StringTableUTF8::TRAFFIC_DEU),
    pp(TrafficDataTypes::DEX, StringTableUTF8::TRAFFIC_DEX),
    pp(TrafficDataTypes::DIP, StringTableUTF8::TRAFFIC_DIP),
    pp(TrafficDataTypes::DLL, StringTableUTF8::TRAFFIC_DLL),
    pp(TrafficDataTypes::DLY, StringTableUTF8::TRAFFIC_DLY),
    pp(TrafficDataTypes::DO,  StringTableUTF8::TRAFFIC_DO ),     
    pp(TrafficDataTypes::DPN, StringTableUTF8::TRAFFIC_DPN),
    pp(TrafficDataTypes::DUD, StringTableUTF8::TRAFFIC_DUD),
    pp(TrafficDataTypes::DVL, StringTableUTF8::TRAFFIC_DVL),
    pp(TrafficDataTypes::DXX, StringTableUTF8::TRAFFIC_DXX),
    pp(TrafficDataTypes::EAM, StringTableUTF8::TRAFFIC_EAM),
    pp(TrafficDataTypes::EBA, StringTableUTF8::TRAFFIC_EBA),
    pp(TrafficDataTypes::EBF, StringTableUTF8::TRAFFIC_EBF),
    pp(TrafficDataTypes::EBG, StringTableUTF8::TRAFFIC_EBG),
    pp(TrafficDataTypes::EBT, StringTableUTF8::TRAFFIC_EBT),
    pp(TrafficDataTypes::ECL, StringTableUTF8::TRAFFIC_ECL),
    pp(TrafficDataTypes::ECM, StringTableUTF8::TRAFFIC_ECM),
    pp(TrafficDataTypes::ECR, StringTableUTF8::TRAFFIC_ECR),
    pp(TrafficDataTypes::ECY, StringTableUTF8::TRAFFIC_ECY),
    pp(TrafficDataTypes::EFA, StringTableUTF8::TRAFFIC_EFA),
    pp(TrafficDataTypes::EFM, StringTableUTF8::TRAFFIC_EFM),
    pp(TrafficDataTypes::EFR, StringTableUTF8::TRAFFIC_EFR),
    pp(TrafficDataTypes::EGT, StringTableUTF8::TRAFFIC_EGT),
    pp(TrafficDataTypes::EHL, StringTableUTF8::TRAFFIC_EHL),
    pp(TrafficDataTypes::EIS, StringTableUTF8::TRAFFIC_EIS),
    pp(TrafficDataTypes::EMH, StringTableUTF8::TRAFFIC_EMH),
    pp(TrafficDataTypes::EMR, StringTableUTF8::TRAFFIC_EMR),
    pp(TrafficDataTypes::EMT, StringTableUTF8::TRAFFIC_EMT),
    pp(TrafficDataTypes::EMV, StringTableUTF8::TRAFFIC_EMV),
    pp(TrafficDataTypes::EMX, StringTableUTF8::TRAFFIC_EMX),
    pp(TrafficDataTypes::EPD, StringTableUTF8::TRAFFIC_EPD),
    pp(TrafficDataTypes::EPR, StringTableUTF8::TRAFFIC_EPR),
    pp(TrafficDataTypes::EQD, StringTableUTF8::TRAFFIC_EQD),
    pp(TrafficDataTypes::ERA, StringTableUTF8::TRAFFIC_ERA),
    pp(TrafficDataTypes::ESA, StringTableUTF8::TRAFFIC_ESA),
    pp(TrafficDataTypes::ESH, StringTableUTF8::TRAFFIC_ESH),
    pp(TrafficDataTypes::ESI, StringTableUTF8::TRAFFIC_ESI),
    pp(TrafficDataTypes::ESJ, StringTableUTF8::TRAFFIC_ESJ),
    pp(TrafficDataTypes::ESM, StringTableUTF8::TRAFFIC_ESM),
    pp(TrafficDataTypes::ESO, StringTableUTF8::TRAFFIC_ESO),
    pp(TrafficDataTypes::ESP, StringTableUTF8::TRAFFIC_ESP),
    pp(TrafficDataTypes::ESS, StringTableUTF8::TRAFFIC_ESS),
    pp(TrafficDataTypes::EST, StringTableUTF8::TRAFFIC_EST),
    pp(TrafficDataTypes::ESX, StringTableUTF8::TRAFFIC_ESX),
    pp(TrafficDataTypes::ESY, StringTableUTF8::TRAFFIC_ESY),
    pp(TrafficDataTypes::ETN, StringTableUTF8::TRAFFIC_ETN),
    pp(TrafficDataTypes::ETO, StringTableUTF8::TRAFFIC_ETO),
    pp(TrafficDataTypes::ETT, StringTableUTF8::TRAFFIC_ETT),
    pp(TrafficDataTypes::EVA, StringTableUTF8::TRAFFIC_EVA),
    pp(TrafficDataTypes::EVC, StringTableUTF8::TRAFFIC_EVC),
    pp(TrafficDataTypes::EVD, StringTableUTF8::TRAFFIC_EVD),
    pp(TrafficDataTypes::EVF, StringTableUTF8::TRAFFIC_EVF),
    pp(TrafficDataTypes::EVM, StringTableUTF8::TRAFFIC_EVM),
    pp(TrafficDataTypes::EVP, StringTableUTF8::TRAFFIC_EVP),
    pp(TrafficDataTypes::EVX, StringTableUTF8::TRAFFIC_EVX),
    pp(TrafficDataTypes::EWS, StringTableUTF8::TRAFFIC_EWS),
    pp(TrafficDataTypes::EWT, StringTableUTF8::TRAFFIC_EWT),
    pp(TrafficDataTypes::EXB, StringTableUTF8::TRAFFIC_EXB),
    pp(TrafficDataTypes::EXC, StringTableUTF8::TRAFFIC_EXC),
    pp(TrafficDataTypes::EXS, StringTableUTF8::TRAFFIC_EXS),
    pp(TrafficDataTypes::FFX, StringTableUTF8::TRAFFIC_FFX),
    pp(TrafficDataTypes::FIG, StringTableUTF8::TRAFFIC_FIG),
    pp(TrafficDataTypes::FIR, StringTableUTF8::TRAFFIC_FIR),
    pp(TrafficDataTypes::FLD, StringTableUTF8::TRAFFIC_FLD),
    pp(TrafficDataTypes::FLF, StringTableUTF8::TRAFFIC_FLF),
    pp(TrafficDataTypes::FLO, StringTableUTF8::TRAFFIC_FLO),
    pp(TrafficDataTypes::FLT, StringTableUTF8::TRAFFIC_FLT),
    pp(TrafficDataTypes::FOD, StringTableUTF8::TRAFFIC_FOD),
    pp(TrafficDataTypes::FOF, StringTableUTF8::TRAFFIC_FOF),
    pp(TrafficDataTypes::FOG, StringTableUTF8::TRAFFIC_FOG),
    pp(TrafficDataTypes::FOP, StringTableUTF8::TRAFFIC_FOP),
    pp(TrafficDataTypes::FOX, StringTableUTF8::TRAFFIC_FOX),
    pp(TrafficDataTypes::FPC, StringTableUTF8::TRAFFIC_FPC),
    pp(TrafficDataTypes::FRH, StringTableUTF8::TRAFFIC_FRH),
    pp(TrafficDataTypes::FRO, StringTableUTF8::TRAFFIC_FRO),
    pp(TrafficDataTypes::FSN, StringTableUTF8::TRAFFIC_FSN),
    pp(TrafficDataTypes::FUE, StringTableUTF8::TRAFFIC_FUE),
    pp(TrafficDataTypes::FUN, StringTableUTF8::TRAFFIC_FUN),
    pp(TrafficDataTypes::GAL, StringTableUTF8::TRAFFIC_GAL),
    pp(TrafficDataTypes::GAS, StringTableUTF8::TRAFFIC_GAS),
    pp(TrafficDataTypes::GMW, StringTableUTF8::TRAFFIC_GMW),
    pp(TrafficDataTypes::GP,  StringTableUTF8::TRAFFIC_GP ),     
    pp(TrafficDataTypes::GUN, StringTableUTF8::TRAFFIC_GUN),
    pp(TrafficDataTypes::HAD, StringTableUTF8::TRAFFIC_HAD),
    pp(TrafficDataTypes::HAI, StringTableUTF8::TRAFFIC_HAI),
    pp(TrafficDataTypes::HAX, StringTableUTF8::TRAFFIC_HAX),
    pp(TrafficDataTypes::HAZ, StringTableUTF8::TRAFFIC_HAZ),
    pp(TrafficDataTypes::HBD, StringTableUTF8::TRAFFIC_HBD),
    pp(TrafficDataTypes::HLL, StringTableUTF8::TRAFFIC_HLL),
    pp(TrafficDataTypes::HLT, StringTableUTF8::TRAFFIC_HLT),
    pp(TrafficDataTypes::HSC, StringTableUTF8::TRAFFIC_HSC),
    pp(TrafficDataTypes::HUR, StringTableUTF8::TRAFFIC_HUR),
    pp(TrafficDataTypes::IAV, StringTableUTF8::TRAFFIC_IAV),
    pp(TrafficDataTypes::IBU, StringTableUTF8::TRAFFIC_IBU),
    pp(TrafficDataTypes::ICP, StringTableUTF8::TRAFFIC_ICP),
    pp(TrafficDataTypes::IMA, StringTableUTF8::TRAFFIC_IMA),
    pp(TrafficDataTypes::IMP, StringTableUTF8::TRAFFIC_IMP),
    pp(TrafficDataTypes::INS, StringTableUTF8::TRAFFIC_INS),
    pp(TrafficDataTypes::IVD, StringTableUTF8::TRAFFIC_IVD),
    pp(TrafficDataTypes::LAP, StringTableUTF8::TRAFFIC_LAP),
    pp(TrafficDataTypes::LB1, StringTableUTF8::TRAFFIC_LB1),
    pp(TrafficDataTypes::LB2, StringTableUTF8::TRAFFIC_LB2),
    pp(TrafficDataTypes::LB3, StringTableUTF8::TRAFFIC_LB3),
    pp(TrafficDataTypes::LBA, StringTableUTF8::TRAFFIC_LBA),
    pp(TrafficDataTypes::LBB, StringTableUTF8::TRAFFIC_LBB),
    pp(TrafficDataTypes::LBC, StringTableUTF8::TRAFFIC_LBC),
    pp(TrafficDataTypes::LBD, StringTableUTF8::TRAFFIC_LBD),
    pp(TrafficDataTypes::LBE, StringTableUTF8::TRAFFIC_LBE),
    pp(TrafficDataTypes::LBH, StringTableUTF8::TRAFFIC_LBH),
    pp(TrafficDataTypes::LBK, StringTableUTF8::TRAFFIC_LBK),
    pp(TrafficDataTypes::LBL, StringTableUTF8::TRAFFIC_LBL),
    pp(TrafficDataTypes::LBP, StringTableUTF8::TRAFFIC_LBP),
    pp(TrafficDataTypes::LBR, StringTableUTF8::TRAFFIC_LBR),
    pp(TrafficDataTypes::LBT, StringTableUTF8::TRAFFIC_LBT),
    pp(TrafficDataTypes::LBV, StringTableUTF8::TRAFFIC_LBV),
    pp(TrafficDataTypes::LBX, StringTableUTF8::TRAFFIC_LBX),
    pp(TrafficDataTypes::LC1, StringTableUTF8::TRAFFIC_LC1),
    pp(TrafficDataTypes::LC2, StringTableUTF8::TRAFFIC_LC2),
    pp(TrafficDataTypes::LC3, StringTableUTF8::TRAFFIC_LC3),
    pp(TrafficDataTypes::LCA, StringTableUTF8::TRAFFIC_LCA),
    pp(TrafficDataTypes::LCB, StringTableUTF8::TRAFFIC_LCB),
    pp(TrafficDataTypes::LCC, StringTableUTF8::TRAFFIC_LCC),
    pp(TrafficDataTypes::LCD, StringTableUTF8::TRAFFIC_LCD),
    pp(TrafficDataTypes::LCE, StringTableUTF8::TRAFFIC_LCE),
    pp(TrafficDataTypes::LCF, StringTableUTF8::TRAFFIC_LCF),
    pp(TrafficDataTypes::LCH, StringTableUTF8::TRAFFIC_LCH),
    pp(TrafficDataTypes::LCI, StringTableUTF8::TRAFFIC_LCI),
    pp(TrafficDataTypes::LCL, StringTableUTF8::TRAFFIC_LCL),
    pp(TrafficDataTypes::LCP, StringTableUTF8::TRAFFIC_LCP),
    pp(TrafficDataTypes::LCR, StringTableUTF8::TRAFFIC_LCR),
    pp(TrafficDataTypes::LCS, StringTableUTF8::TRAFFIC_LCS),
    pp(TrafficDataTypes::LCT, StringTableUTF8::TRAFFIC_LCT),
    pp(TrafficDataTypes::LCV, StringTableUTF8::TRAFFIC_LCV),
    pp(TrafficDataTypes::LCW, StringTableUTF8::TRAFFIC_LCW),
    pp(TrafficDataTypes::LLB, StringTableUTF8::TRAFFIC_LLB),
    pp(TrafficDataTypes::LLC, StringTableUTF8::TRAFFIC_LLC),
    pp(TrafficDataTypes::LLD, StringTableUTF8::TRAFFIC_LLD),
    pp(TrafficDataTypes::LLL, StringTableUTF8::TRAFFIC_LLL),
    pp(TrafficDataTypes::LLT, StringTableUTF8::TRAFFIC_LLT),
    pp(TrafficDataTypes::LO1, StringTableUTF8::TRAFFIC_LO1),
    pp(TrafficDataTypes::LO2, StringTableUTF8::TRAFFIC_LO2),
    pp(TrafficDataTypes::LO3, StringTableUTF8::TRAFFIC_LO3),
    pp(TrafficDataTypes::LPB, StringTableUTF8::TRAFFIC_LPB),
    pp(TrafficDataTypes::LPC, StringTableUTF8::TRAFFIC_LPC),
    pp(TrafficDataTypes::LPX, StringTableUTF8::TRAFFIC_LPX),
    pp(TrafficDataTypes::LRR, StringTableUTF8::TRAFFIC_LRR),
    pp(TrafficDataTypes::LRS, StringTableUTF8::TRAFFIC_LRS),
    pp(TrafficDataTypes::LRX, StringTableUTF8::TRAFFIC_LRX),
    pp(TrafficDataTypes::LS1, StringTableUTF8::TRAFFIC_LS1),
    pp(TrafficDataTypes::LS2, StringTableUTF8::TRAFFIC_LS2),
    pp(TrafficDataTypes::LS3, StringTableUTF8::TRAFFIC_LS3),
    pp(TrafficDataTypes::LS4, StringTableUTF8::TRAFFIC_LS4),
    pp(TrafficDataTypes::LS5, StringTableUTF8::TRAFFIC_LS5),
    pp(TrafficDataTypes::LS6, StringTableUTF8::TRAFFIC_LS6),
    pp(TrafficDataTypes::LSG, StringTableUTF8::TRAFFIC_LSG),
    pp(TrafficDataTypes::LSL, StringTableUTF8::TRAFFIC_LSL),
    pp(TrafficDataTypes::LSO, StringTableUTF8::TRAFFIC_LSO),
    pp(TrafficDataTypes::LSR, StringTableUTF8::TRAFFIC_LSR),
    pp(TrafficDataTypes::LTH, StringTableUTF8::TRAFFIC_LTH),
    pp(TrafficDataTypes::LTM, StringTableUTF8::TRAFFIC_LTM),
    pp(TrafficDataTypes::LVB, StringTableUTF8::TRAFFIC_LVB),
    pp(TrafficDataTypes::MAR, StringTableUTF8::TRAFFIC_MAR),
    pp(TrafficDataTypes::MHR, StringTableUTF8::TRAFFIC_MHR),
    pp(TrafficDataTypes::MIL, StringTableUTF8::TRAFFIC_MIL),
    pp(TrafficDataTypes::MKT, StringTableUTF8::TRAFFIC_MKT),
    pp(TrafficDataTypes::MSR, StringTableUTF8::TRAFFIC_MSR),
    pp(TrafficDataTypes::MUD, StringTableUTF8::TRAFFIC_MUD),
    pp(TrafficDataTypes::MVL, StringTableUTF8::TRAFFIC_MVL),
    pp(TrafficDataTypes::NAR, StringTableUTF8::TRAFFIC_NAR),
    pp(TrafficDataTypes::NCR, StringTableUTF8::TRAFFIC_NCR),
    pp(TrafficDataTypes::NDT, StringTableUTF8::TRAFFIC_NDT),
    pp(TrafficDataTypes::NIA, StringTableUTF8::TRAFFIC_NIA),
    pp(TrafficDataTypes::NLS, StringTableUTF8::TRAFFIC_NLS),
    pp(TrafficDataTypes::NML, StringTableUTF8::TRAFFIC_NML),
    pp(TrafficDataTypes::NOO, StringTableUTF8::TRAFFIC_NOO),
    pp(TrafficDataTypes::NRL, StringTableUTF8::TRAFFIC_NRL),
    pp(TrafficDataTypes::NUL, StringTableUTF8::TRAFFIC_NUL),
    pp(TrafficDataTypes::OBR, StringTableUTF8::TRAFFIC_OBR),
    pp(TrafficDataTypes::OCC, StringTableUTF8::TRAFFIC_OCC),
    pp(TrafficDataTypes::OCL, StringTableUTF8::TRAFFIC_OCL),
    pp(TrafficDataTypes::OHV, StringTableUTF8::TRAFFIC_OHV),
    pp(TrafficDataTypes::OHW, StringTableUTF8::TRAFFIC_OHW),
    pp(TrafficDataTypes::OHX, StringTableUTF8::TRAFFIC_OHX),
    pp(TrafficDataTypes::OIL, StringTableUTF8::TRAFFIC_OIL),
    pp(TrafficDataTypes::OMV, StringTableUTF8::TRAFFIC_OMV),
    pp(TrafficDataTypes::OPE, StringTableUTF8::TRAFFIC_OPE),
    pp(TrafficDataTypes::OSI, StringTableUTF8::TRAFFIC_OSI),
    pp(TrafficDataTypes::OWW, StringTableUTF8::TRAFFIC_OWW),
    pp(TrafficDataTypes::PAC, StringTableUTF8::TRAFFIC_PAC),
    pp(TrafficDataTypes::PCB, StringTableUTF8::TRAFFIC_PCB),
    pp(TrafficDataTypes::PCC, StringTableUTF8::TRAFFIC_PCC),
    pp(TrafficDataTypes::PEO, StringTableUTF8::TRAFFIC_PEO),
    pp(TrafficDataTypes::PFN, StringTableUTF8::TRAFFIC_PFN),
    pp(TrafficDataTypes::PFS, StringTableUTF8::TRAFFIC_PFS),
    pp(TrafficDataTypes::PIX, StringTableUTF8::TRAFFIC_PIX),
    pp(TrafficDataTypes::PKO, StringTableUTF8::TRAFFIC_PKO),
    pp(TrafficDataTypes::PKT, StringTableUTF8::TRAFFIC_PKT),
    pp(TrafficDataTypes::PMN, StringTableUTF8::TRAFFIC_PMN),
    pp(TrafficDataTypes::PMS, StringTableUTF8::TRAFFIC_PMS),
    pp(TrafficDataTypes::PRA, StringTableUTF8::TRAFFIC_PRA),
    pp(TrafficDataTypes::PRC, StringTableUTF8::TRAFFIC_PRC),
    pp(TrafficDataTypes::PRI, StringTableUTF8::TRAFFIC_PRI),
    pp(TrafficDataTypes::PRL, StringTableUTF8::TRAFFIC_PRL),
    pp(TrafficDataTypes::PRN, StringTableUTF8::TRAFFIC_PRN),
    pp(TrafficDataTypes::PRO, StringTableUTF8::TRAFFIC_PRO),
    pp(TrafficDataTypes::PRS, StringTableUTF8::TRAFFIC_PRS),
    pp(TrafficDataTypes::PRV, StringTableUTF8::TRAFFIC_PRV),
    pp(TrafficDataTypes::PRX, StringTableUTF8::TRAFFIC_PRX),
    pp(TrafficDataTypes::PSA, StringTableUTF8::TRAFFIC_PSA),
    pp(TrafficDataTypes::PSL, StringTableUTF8::TRAFFIC_PSL),
    pp(TrafficDataTypes::PSN, StringTableUTF8::TRAFFIC_PSN),
    pp(TrafficDataTypes::PSR, StringTableUTF8::TRAFFIC_PSR),
    pp(TrafficDataTypes::PSS, StringTableUTF8::TRAFFIC_PSS),
    pp(TrafficDataTypes::PTB, StringTableUTF8::TRAFFIC_PTB),
    pp(TrafficDataTypes::PTC, StringTableUTF8::TRAFFIC_PTC),
    pp(TrafficDataTypes::PTH, StringTableUTF8::TRAFFIC_PTH),
    pp(TrafficDataTypes::PTN, StringTableUTF8::TRAFFIC_PTN),
    pp(TrafficDataTypes::PTS, StringTableUTF8::TRAFFIC_PTS),
    pp(TrafficDataTypes::RAC, StringTableUTF8::TRAFFIC_RAC),
    pp(TrafficDataTypes::RAD, StringTableUTF8::TRAFFIC_RAD),
    pp(TrafficDataTypes::RAF, StringTableUTF8::TRAFFIC_RAF),
    pp(TrafficDataTypes::RAH, StringTableUTF8::TRAFFIC_RAH),
    pp(TrafficDataTypes::RAI, StringTableUTF8::TRAFFIC_RAI),
    pp(TrafficDataTypes::RBA, StringTableUTF8::TRAFFIC_RBA),
    pp(TrafficDataTypes::RBD, StringTableUTF8::TRAFFIC_RBD),
    pp(TrafficDataTypes::RBE, StringTableUTF8::TRAFFIC_RBE),
    pp(TrafficDataTypes::RBI, StringTableUTF8::TRAFFIC_RBI),
    pp(TrafficDataTypes::RBL, StringTableUTF8::TRAFFIC_RBL),
    pp(TrafficDataTypes::RBM, StringTableUTF8::TRAFFIC_RBM),
    pp(TrafficDataTypes::RBX, StringTableUTF8::TRAFFIC_RBX),
    pp(TrafficDataTypes::RC2, StringTableUTF8::TRAFFIC_RC2),
    pp(TrafficDataTypes::RCA, StringTableUTF8::TRAFFIC_RCA),
    pp(TrafficDataTypes::RCB, StringTableUTF8::TRAFFIC_RCB),
    pp(TrafficDataTypes::RCD, StringTableUTF8::TRAFFIC_RCD),
    pp(TrafficDataTypes::RCE, StringTableUTF8::TRAFFIC_RCE),
    pp(TrafficDataTypes::RCI, StringTableUTF8::TRAFFIC_RCI),
    pp(TrafficDataTypes::RCL, StringTableUTF8::TRAFFIC_RCL),
    pp(TrafficDataTypes::RCP, StringTableUTF8::TRAFFIC_RCP),
    pp(TrafficDataTypes::RCR, StringTableUTF8::TRAFFIC_RCR),
    pp(TrafficDataTypes::RCT, StringTableUTF8::TRAFFIC_RCT),
    pp(TrafficDataTypes::RCW, StringTableUTF8::TRAFFIC_RCW),
    pp(TrafficDataTypes::RCX, StringTableUTF8::TRAFFIC_RCX),
    pp(TrafficDataTypes::RDW, StringTableUTF8::TRAFFIC_RDW),
    pp(TrafficDataTypes::REB, StringTableUTF8::TRAFFIC_REB),
    pp(TrafficDataTypes::REC, StringTableUTF8::TRAFFIC_REC),
    pp(TrafficDataTypes::REL, StringTableUTF8::TRAFFIC_REL),
    pp(TrafficDataTypes::REO, StringTableUTF8::TRAFFIC_REO),
    pp(TrafficDataTypes::RET, StringTableUTF8::TRAFFIC_RET),
    pp(TrafficDataTypes::REW, StringTableUTF8::TRAFFIC_REW),
    pp(TrafficDataTypes::REX, StringTableUTF8::TRAFFIC_REX),
    pp(TrafficDataTypes::RIC, StringTableUTF8::TRAFFIC_RIC),
    pp(TrafficDataTypes::RIN, StringTableUTF8::TRAFFIC_RIN),
    pp(TrafficDataTypes::RIS, StringTableUTF8::TRAFFIC_RIS),
    pp(TrafficDataTypes::RLS, StringTableUTF8::TRAFFIC_RLS),
    pp(TrafficDataTypes::RLU, StringTableUTF8::TRAFFIC_RLU),
    pp(TrafficDataTypes::RMD, StringTableUTF8::TRAFFIC_RMD),
    pp(TrafficDataTypes::RMK, StringTableUTF8::TRAFFIC_RMK),
    pp(TrafficDataTypes::RMM, StringTableUTF8::TRAFFIC_RMM),
    pp(TrafficDataTypes::RMP, StringTableUTF8::TRAFFIC_RMP),
    pp(TrafficDataTypes::RMV, StringTableUTF8::TRAFFIC_RMV),
    pp(TrafficDataTypes::RMW, StringTableUTF8::TRAFFIC_RMW),
    pp(TrafficDataTypes::RMX, StringTableUTF8::TRAFFIC_RMX),
    pp(TrafficDataTypes::RNL, StringTableUTF8::TRAFFIC_RNL),
    pp(TrafficDataTypes::RNW, StringTableUTF8::TRAFFIC_RNW),
    pp(TrafficDataTypes::ROA, StringTableUTF8::TRAFFIC_ROA),
    pp(TrafficDataTypes::ROC, StringTableUTF8::TRAFFIC_ROC),
    pp(TrafficDataTypes::RPB, StringTableUTF8::TRAFFIC_RPB),
    pp(TrafficDataTypes::RPC, StringTableUTF8::TRAFFIC_RPC),
    pp(TrafficDataTypes::RRI, StringTableUTF8::TRAFFIC_RRI),
    pp(TrafficDataTypes::RRU, StringTableUTF8::TRAFFIC_RRU),
    pp(TrafficDataTypes::RRW, StringTableUTF8::TRAFFIC_RRW),
    pp(TrafficDataTypes::RSB, StringTableUTF8::TRAFFIC_RSB),
    pp(TrafficDataTypes::RSI, StringTableUTF8::TRAFFIC_RSI),
    pp(TrafficDataTypes::RSL, StringTableUTF8::TRAFFIC_RSL),
    pp(TrafficDataTypes::RSN, StringTableUTF8::TRAFFIC_RSN),
    pp(TrafficDataTypes::RSO, StringTableUTF8::TRAFFIC_RSO),
    pp(TrafficDataTypes::RSR, StringTableUTF8::TRAFFIC_RSR),
    pp(TrafficDataTypes::RTO, StringTableUTF8::TRAFFIC_RTO),
    pp(TrafficDataTypes::RWC, StringTableUTF8::TRAFFIC_RWC),
    pp(TrafficDataTypes::RWI, StringTableUTF8::TRAFFIC_RWI),
    pp(TrafficDataTypes::RWK, StringTableUTF8::TRAFFIC_RWK),
    pp(TrafficDataTypes::RWL, StringTableUTF8::TRAFFIC_RWL),
    pp(TrafficDataTypes::RWM, StringTableUTF8::TRAFFIC_RWM),
    pp(TrafficDataTypes::RWR, StringTableUTF8::TRAFFIC_RWR),
    pp(TrafficDataTypes::RWX, StringTableUTF8::TRAFFIC_RWX),
    pp(TrafficDataTypes::RXB, StringTableUTF8::TRAFFIC_RXB),
    pp(TrafficDataTypes::RXC, StringTableUTF8::TRAFFIC_RXC),
    pp(TrafficDataTypes::RXD, StringTableUTF8::TRAFFIC_RXD),
    pp(TrafficDataTypes::RXO, StringTableUTF8::TRAFFIC_RXO),
    pp(TrafficDataTypes::SAB, StringTableUTF8::TRAFFIC_SAB),
    pp(TrafficDataTypes::SAL, StringTableUTF8::TRAFFIC_SAL),
    pp(TrafficDataTypes::SAN, StringTableUTF8::TRAFFIC_SAN),
    pp(TrafficDataTypes::SAO, StringTableUTF8::TRAFFIC_SAO),
    pp(TrafficDataTypes::SAT, StringTableUTF8::TRAFFIC_SAT),
    pp(TrafficDataTypes::SCI, StringTableUTF8::TRAFFIC_SCI),
    pp(TrafficDataTypes::SCN, StringTableUTF8::TRAFFIC_SCN),
    pp(TrafficDataTypes::SEW, StringTableUTF8::TRAFFIC_SEW),
    pp(TrafficDataTypes::SEX, StringTableUTF8::TRAFFIC_SEX),
    pp(TrafficDataTypes::SFB, StringTableUTF8::TRAFFIC_SFB),
    pp(TrafficDataTypes::SFC, StringTableUTF8::TRAFFIC_SFC),
    pp(TrafficDataTypes::SFH, StringTableUTF8::TRAFFIC_SFH),
    pp(TrafficDataTypes::SFL, StringTableUTF8::TRAFFIC_SFL),
    pp(TrafficDataTypes::SFO, StringTableUTF8::TRAFFIC_SFO),
    pp(TrafficDataTypes::SHL, StringTableUTF8::TRAFFIC_SHL),
    pp(TrafficDataTypes::SHX, StringTableUTF8::TRAFFIC_SHX),
    pp(TrafficDataTypes::SLT, StringTableUTF8::TRAFFIC_SLT),
    pp(TrafficDataTypes::SLU, StringTableUTF8::TRAFFIC_SLU),
    pp(TrafficDataTypes::SM,  StringTableUTF8::TRAFFIC_SM ),     
    pp(TrafficDataTypes::SMC, StringTableUTF8::TRAFFIC_SMC),
    pp(TrafficDataTypes::SMG, StringTableUTF8::TRAFFIC_SMG),
    pp(TrafficDataTypes::SMO, StringTableUTF8::TRAFFIC_SMO),
    pp(TrafficDataTypes::SMV, StringTableUTF8::TRAFFIC_SMV),
    pp(TrafficDataTypes::SMX, StringTableUTF8::TRAFFIC_SMX),
    pp(TrafficDataTypes::SN,  StringTableUTF8::TRAFFIC_SN ),     
    pp(TrafficDataTypes::SND, StringTableUTF8::TRAFFIC_SND),
    pp(TrafficDataTypes::SNF, StringTableUTF8::TRAFFIC_SNF),
    pp(TrafficDataTypes::SNP, StringTableUTF8::TRAFFIC_SNP),
    pp(TrafficDataTypes::SNR, StringTableUTF8::TRAFFIC_SNR),
    pp(TrafficDataTypes::SNS, StringTableUTF8::TRAFFIC_SNS),
    pp(TrafficDataTypes::SNW, StringTableUTF8::TRAFFIC_SNW),
    pp(TrafficDataTypes::SNX, StringTableUTF8::TRAFFIC_SNX),
    pp(TrafficDataTypes::SPC, StringTableUTF8::TRAFFIC_SPC),
    pp(TrafficDataTypes::SPL, StringTableUTF8::TRAFFIC_SPL),
    pp(TrafficDataTypes::SPY, StringTableUTF8::TRAFFIC_SPY),
    pp(TrafficDataTypes::SR,  StringTableUTF8::TRAFFIC_SR ),     
    pp(TrafficDataTypes::SRC, StringTableUTF8::TRAFFIC_SRC),
    pp(TrafficDataTypes::SRO, StringTableUTF8::TRAFFIC_SRO),
    pp(TrafficDataTypes::SRX, StringTableUTF8::TRAFFIC_SRX),
    pp(TrafficDataTypes::SSF, StringTableUTF8::TRAFFIC_SSF),
    pp(TrafficDataTypes::SSM, StringTableUTF8::TRAFFIC_SSM),
    pp(TrafficDataTypes::SSO, StringTableUTF8::TRAFFIC_SSO),
    pp(TrafficDataTypes::STD, StringTableUTF8::TRAFFIC_STD),
    pp(TrafficDataTypes::STF, StringTableUTF8::TRAFFIC_STF),
    pp(TrafficDataTypes::STI, StringTableUTF8::TRAFFIC_STI),
    pp(TrafficDataTypes::STM, StringTableUTF8::TRAFFIC_STM),
    pp(TrafficDataTypes::STU, StringTableUTF8::TRAFFIC_STU),
    pp(TrafficDataTypes::SUB, StringTableUTF8::TRAFFIC_SUB),
    pp(TrafficDataTypes::SVH, StringTableUTF8::TRAFFIC_SVH),
    pp(TrafficDataTypes::SWA, StringTableUTF8::TRAFFIC_SWA),
    pp(TrafficDataTypes::SWC, StringTableUTF8::TRAFFIC_SWC),
    pp(TrafficDataTypes::SWH, StringTableUTF8::TRAFFIC_SWH),
    pp(TrafficDataTypes::SWI, StringTableUTF8::TRAFFIC_SWI),
    pp(TrafficDataTypes::SWN, StringTableUTF8::TRAFFIC_SWN),
    pp(TrafficDataTypes::SWS, StringTableUTF8::TRAFFIC_SWS),
    pp(TrafficDataTypes::SWT, StringTableUTF8::TRAFFIC_SWT),
    pp(TrafficDataTypes::TAL, StringTableUTF8::TRAFFIC_TAL),
    pp(TrafficDataTypes::TAX, StringTableUTF8::TRAFFIC_TAX),
    pp(TrafficDataTypes::TBU, StringTableUTF8::TRAFFIC_TBU),
    pp(TrafficDataTypes::TCC, StringTableUTF8::TRAFFIC_TCC),
    pp(TrafficDataTypes::TCN, StringTableUTF8::TRAFFIC_TCN),
    pp(TrafficDataTypes::TCX, StringTableUTF8::TRAFFIC_TCX),
    pp(TrafficDataTypes::TDX, StringTableUTF8::TRAFFIC_TDX),
    pp(TrafficDataTypes::TEA, StringTableUTF8::TRAFFIC_TEA),
    pp(TrafficDataTypes::TEX, StringTableUTF8::TRAFFIC_TEX),
    pp(TrafficDataTypes::TFA, StringTableUTF8::TRAFFIC_TFA),
    pp(TrafficDataTypes::TGL, StringTableUTF8::TRAFFIC_TGL),
    pp(TrafficDataTypes::TGW, StringTableUTF8::TRAFFIC_TGW),
    pp(TrafficDataTypes::THU, StringTableUTF8::TRAFFIC_THU),
    pp(TrafficDataTypes::TIA, StringTableUTF8::TRAFFIC_TIA),
    pp(TrafficDataTypes::TIR, StringTableUTF8::TRAFFIC_TIR),
    pp(TrafficDataTypes::TLI, StringTableUTF8::TRAFFIC_TLI),
    pp(TrafficDataTypes::TLN, StringTableUTF8::TRAFFIC_TLN),
    pp(TrafficDataTypes::TLO, StringTableUTF8::TRAFFIC_TLO),
    pp(TrafficDataTypes::TLT, StringTableUTF8::TRAFFIC_TLT),
    pp(TrafficDataTypes::TLV, StringTableUTF8::TRAFFIC_TLV),
    pp(TrafficDataTypes::TLX, StringTableUTF8::TRAFFIC_TLX),
    pp(TrafficDataTypes::TM,  StringTableUTF8::TRAFFIC_TM ),     
    pp(TrafficDataTypes::TMO, StringTableUTF8::TRAFFIC_TMO),
    pp(TrafficDataTypes::TMP, StringTableUTF8::TRAFFIC_TMP),
    pp(TrafficDataTypes::TNL, StringTableUTF8::TRAFFIC_TNL),
    pp(TrafficDataTypes::TNW, StringTableUTF8::TRAFFIC_TNW),
    pp(TrafficDataTypes::TOR, StringTableUTF8::TRAFFIC_TOR),
    pp(TrafficDataTypes::TOX, StringTableUTF8::TRAFFIC_TOX),
    pp(TrafficDataTypes::TR,  StringTableUTF8::TRAFFIC_TR ),     
    pp(TrafficDataTypes::TRA, StringTableUTF8::TRAFFIC_TRA),
    pp(TrafficDataTypes::TRC, StringTableUTF8::TRAFFIC_TRC),
    pp(TrafficDataTypes::TRT, StringTableUTF8::TRAFFIC_TRT),
    pp(TrafficDataTypes::TRX, StringTableUTF8::TRAFFIC_TRX),
    pp(TrafficDataTypes::TSC, StringTableUTF8::TRAFFIC_TSC),
    pp(TrafficDataTypes::TSR, StringTableUTF8::TRAFFIC_TSR),
    pp(TrafficDataTypes::TSX, StringTableUTF8::TRAFFIC_TSX),
    pp(TrafficDataTypes::TTB, StringTableUTF8::TRAFFIC_TTB),
    pp(TrafficDataTypes::TTL, StringTableUTF8::TRAFFIC_TTL),
    pp(TrafficDataTypes::TTM, StringTableUTF8::TRAFFIC_TTM),
    pp(TrafficDataTypes::TUB, StringTableUTF8::TRAFFIC_TUB),
    pp(TrafficDataTypes::TUC, StringTableUTF8::TRAFFIC_TUC),
    pp(TrafficDataTypes::TVX, StringTableUTF8::TRAFFIC_TVX),
    pp(TrafficDataTypes::TWI, StringTableUTF8::TRAFFIC_TWI),
    pp(TrafficDataTypes::TXA, StringTableUTF8::TRAFFIC_TXA),
    pp(TrafficDataTypes::TXB, StringTableUTF8::TRAFFIC_TXB),
    pp(TrafficDataTypes::TXC, StringTableUTF8::TRAFFIC_TXC),
    pp(TrafficDataTypes::TXD, StringTableUTF8::TRAFFIC_TXD),
    pp(TrafficDataTypes::TXL, StringTableUTF8::TRAFFIC_TXL),
    pp(TrafficDataTypes::TXN, StringTableUTF8::TRAFFIC_TXN),
    pp(TrafficDataTypes::TXO, StringTableUTF8::TRAFFIC_TXO),
    pp(TrafficDataTypes::TXP, StringTableUTF8::TRAFFIC_TXP),
    pp(TrafficDataTypes::TXR, StringTableUTF8::TRAFFIC_TXR),
    pp(TrafficDataTypes::TXS, StringTableUTF8::TRAFFIC_TXS),
    pp(TrafficDataTypes::TXV, StringTableUTF8::TRAFFIC_TXV),
    pp(TrafficDataTypes::TXW, StringTableUTF8::TRAFFIC_TXW),
    pp(TrafficDataTypes::TXX, StringTableUTF8::TRAFFIC_TXX),
    pp(TrafficDataTypes::TXY, StringTableUTF8::TRAFFIC_TXY),
    pp(TrafficDataTypes::TXZ, StringTableUTF8::TRAFFIC_TXZ),
    pp(TrafficDataTypes::UBA, StringTableUTF8::TRAFFIC_UBA),
    pp(TrafficDataTypes::UBV, StringTableUTF8::TRAFFIC_UBV),
    pp(TrafficDataTypes::UCA, StringTableUTF8::TRAFFIC_UCA),
    pp(TrafficDataTypes::UGI, StringTableUTF8::TRAFFIC_UGI),
    pp(TrafficDataTypes::UHA, StringTableUTF8::TRAFFIC_UHA),
    pp(TrafficDataTypes::UHV, StringTableUTF8::TRAFFIC_UHV),
    pp(TrafficDataTypes::USI, StringTableUTF8::TRAFFIC_USI),
    pp(TrafficDataTypes::USN, StringTableUTF8::TRAFFIC_USN),
    pp(TrafficDataTypes::VFR, StringTableUTF8::TRAFFIC_VFR),
    pp(TrafficDataTypes::VIR, StringTableUTF8::TRAFFIC_VIR),
    pp(TrafficDataTypes::VNW, StringTableUTF8::TRAFFIC_VNW),
    pp(TrafficDataTypes::VOD, StringTableUTF8::TRAFFIC_VOD),
    pp(TrafficDataTypes::VSA, StringTableUTF8::TRAFFIC_VSA),
    pp(TrafficDataTypes::VSM, StringTableUTF8::TRAFFIC_VSM),
    pp(TrafficDataTypes::VSX, StringTableUTF8::TRAFFIC_VSX),
    pp(TrafficDataTypes::VWC, StringTableUTF8::TRAFFIC_VWC),
    pp(TrafficDataTypes::VWI, StringTableUTF8::TRAFFIC_VWI),
    pp(TrafficDataTypes::VWN, StringTableUTF8::TRAFFIC_VWN),
    pp(TrafficDataTypes::VWX, StringTableUTF8::TRAFFIC_VWX),
    pp(TrafficDataTypes::WBC, StringTableUTF8::TRAFFIC_WBC),
    pp(TrafficDataTypes::WBS, StringTableUTF8::TRAFFIC_WBS),
    pp(TrafficDataTypes::WHI, StringTableUTF8::TRAFFIC_WHI),
    pp(TrafficDataTypes::WIC, StringTableUTF8::TRAFFIC_WIC),
    pp(TrafficDataTypes::WIG, StringTableUTF8::TRAFFIC_WIG),
    pp(TrafficDataTypes::WIS, StringTableUTF8::TRAFFIC_WIS),
    pp(TrafficDataTypes::WIX, StringTableUTF8::TRAFFIC_WIX),
    pp(TrafficDataTypes::WLD, StringTableUTF8::TRAFFIC_WLD),
    pp(TrafficDataTypes::WLL, StringTableUTF8::TRAFFIC_WLL),
    pp(TrafficDataTypes::WLT, StringTableUTF8::TRAFFIC_WLT),
    pp(TrafficDataTypes::WMW, StringTableUTF8::TRAFFIC_WMW),
    pp(TrafficDataTypes::WR,  StringTableUTF8::TRAFFIC_WR ),     
    pp(TrafficDataTypes::WRM, StringTableUTF8::TRAFFIC_WRM),
    pp(TrafficDataTypes::WST, StringTableUTF8::TRAFFIC_WST),
    pp(TrafficDataTypes::WSX, StringTableUTF8::TRAFFIC_WSX),
    pp(TrafficDataTypes::WSZ, StringTableUTF8::TRAFFIC_WSZ),
    pp(TrafficDataTypes::NoPhrase, StringTableUTF8::TRAFFIC_LSO),
#endif
      };
      c_phraseStringMap.insert(phraseString_map, 
                               phraseString_map + 
                               sizeof(phraseString_map)/
                               sizeof(*phraseString_map));
   }

   return true;
}

bool InfoDatexUtils::c_initialized = initializeTables();
