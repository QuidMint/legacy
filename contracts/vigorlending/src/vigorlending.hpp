#pragma once
#include <cmath>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/transaction.hpp>

#include "asset_container.hpp"
#include "common.hpp"
#include "croneos.hpp"
#include "daccustodian.hpp"
#include "limits.hpp"
#include "reputation.hpp"
#include "table.hpp"
#include "curve.sx.hpp"

#define EVENT_HANDLER(listen) [[eosio::on_notify(listen)]] void
#define INSURANCE std::string("insurance")
#define COLLATERAL std::string("collateral")
#define VIGFEES std::string("vigfees")
#define SAVINGS std::string("savings")
#define STAKEREFUND std::string("stakerefund")
#define DEBT std::string("debt")
#define L_DEBT std::string("l_debt")
#define VIGOR_SYMBOL eosio::symbol("VIGOR", 4)
#define USDT_SYMBOL eosio::symbol("USDT", 4)
#define VIG4_SYMBOL eosio::symbol("VIG", 4)
#define VIG10_SYMBOL eosio::symbol("VIG", 10)
#define SYSTEM_TOKEN_SYMBOL eosio::symbol("EOS", 4)
#define IQ_TOKEN_SYMBOL eosio::symbol("IQ", 3)
#define PBTC_TOKEN_SYMBOL eosio::symbol("PBTC", 8)
#define PETH_TOKEN_SYMBOL eosio::symbol("PETH", 9)
#define BOX_TOKEN_SYMBOL eosio::symbol("BOX", 6)
#define BOXK1_TOKEN_SYMBOL eosio::symbol("BOXK", 0)
#define BOXBX1_TOKEN_SYMBOL eosio::symbol("BOXBX", 0)
#define BOXL1_TOKEN_SYMBOL eosio::symbol("BOXL", 0)
#define BOXI1_TOKEN_SYMBOL eosio::symbol("BOXI", 0)
#define BOXGL1_TOKEN_SYMBOL eosio::symbol("BOXGL", 0)
#define BOXFU1_TOKEN_SYMBOL eosio::symbol("BOXFU", 0)
#define BOXA1_TOKEN_SYMBOL eosio::symbol("BOXA", 0)
#define BOXAPM1_TOKEN_SYMBOL eosio::symbol("BOXAPM", 0)
#define BOXACK1_TOKEN_SYMBOL eosio::symbol("BOXACK", 0)
#define BOXVP1_TOKEN_SYMBOL eosio::symbol("BOXVP", 0)
#define BOXAI1_TOKEN_SYMBOL eosio::symbol("BOXAI", 0)

#if defined(LOCAL_MODE)
#define CURVESX_CONTRACT eosio::name("vigorsxcurve")
#define CURVELP_TOKEN_CONTRACT eosio::name("dummytokensx")
#define SXE_TOKEN_SYMBOL eosio::symbol("SXE", 4)
#define ZAP_CONTRACT eosio::name("vigorsxzap11")
#else
#define CURVESX_CONTRACT eosio::name("curve.sx")
#define CURVELP_TOKEN_CONTRACT eosio::name("lptoken.sx")
#define SXE_TOKEN_SYMBOL eosio::symbol("SXE", 4)
#define ZAP_CONTRACT eosio::name("zap.sx")
#endif

const double volPrecision = 1000000;
const double corrPrecision = 1000000;
const double pricePrecision = 1000000;
const uint64_t defaultVol = 600000;

CONTRACT vigorlending: public eosio::contract {
 private:
  TABLE user_s {
    eosio::name usern;
    eosio::asset debt = eosio::asset(0, VIGOR_SYMBOL);  // This can be a uint64 because we know the currency already
   protected:
    reputation_s reputation;
    asset_container_t collateral;
    asset_container_t insurance;

   public:
    double valueofcol = 0.0;  // dollar value of collateral
    double valueofins = 0.0;  // dollar value of insurance
    double tesprice = 0.0;    //  %
    double earnrate = 0.0;    //  %
    double pcts = 0.0;        //  %
    double volcol = 1.0;      // volatility of the user collateral portfolio
    double txnvolume = 0.0;   // total dollar amount of outbound transactions for the current day, timer is _g_atimer
    double rewardsave = 0.0;  // Running total of VIG savings rewards gained by this user. If user has VIGOR deposited into savings they receive VIG rewards into vigfees. units: number of VIG
    double prem = 0.0;        // Running total of VIG premiums paid by this user for purchasing insurance for their VIGOR loans. It is paid from this user's vigfees to the lender/insurance pool. units: number of VIG
    double svalueofinsx;
    eosio::time_point lastupdate;
    eosio::asset l_debt = eosio::asset(0, VIGOR_SYMBOL);

   protected:
    asset_container_t l_collateral;

   public:
    double l_valueofcol = 0.0;  // dollar value of l_collateral
    double l_tesprice = 0.0;    // %
    double l_pcts = 0.0;        // %
    double l_volcol = 1.0;      // volatility of the user collateral portfolio
    double savings = 0.0;       // number of VIGOR in savings. VIGOR in savings gets VIG rewards paid from vigordacfund (an expense of the DAC) paid into vigfees. Savers are at risk of taking bailouts, last in line after finalreserve. Savings is the lowest risk activity on vigorlending.
    double rewardrexvot = 0.0;  // last period accumulated rewards paid for all reward types (eg rex, vote proxy, lptoken.defi, txn fees from vigorlending and curve.sx for USDT deposits), in dollars, period defined by atimer (for example 86400 sec)
    double l_prem = 0.0;        // Running total of VIG premiums paid by this user for purchasing insurance for their Crypto loans (not VIGOR). It is paid from this user's vigfees to the lender/insurance pool. units: number of VIG
    double l_svalueofinsx;
    uint64_t vigfees = 0;               // number of VIG that user has in vigfees. borrower fees are deducted from here, insurer/lender rewards are added to here, VIG for savings rewards are added here
    double rewardlend = 0;              // accumulated rewards paid for all reward types (eg rex, vote proxy, lptoken.defi, txn fees from vigorlending and curve.sx for USDT deposits), in dollars, for the period since last atimer rollover (for example 86400 sec)
    eosio::time_point_sec rewardlend2;  // paying insurers VIG on VIG is taboo. if insurer has VIG in insurance then rewards paid to the them into savings are denominated in VIGOR. Note rewardlend2.utc_seconds is type unint32_t, Note also user vigordacfund uses VIG to collateralize a VIGOR borrow
    //display this in the UI
    //     table_name   table_field_name    display_name            data_type       example_value   display_format  description
    //1    user         tesprice            VIGOR borrow rate %     double          0.015           1.50%           Annualized rate that VIGOR borrowers pay in periodic premiums to insure their Crypto Collateral against downside market moves, as a percentage of VIGOR Debt. `. Borrow rates are floating and change continuously over time based on system riskiness as measured by Solvency (down markets).
    //1    user         l_tesprice          Crypto borrow rate %    double          0.015           1.50%           Annualized rate that crypto borrowers pay in periodic premiums to insure their Crypto Debt against upside market moves, as a percentage of the value of the Crypto Debt. The payments are denominated in VIG and deducted automatically throughout the day from the user vigfees. Borrow rates are floating and change continuously over time based on system riskiness as measured by Solvency (up markets).
    //1    user         earnrate            Earn Rate               double          0.07            7.00%           Annualized rate of return earned by insurers for depositing cryptos and/or VIGOR tokens into Insurance. Earnings are paid to insurers automatically throughout the day into their vigfees as periodic premiums denominated in VIG. Earn rates are floating and change continuously over time based on system riskiness as measured by Solvency (up markets) and Solvency (down markets).
    //1    globals      savingsrate         Savings Rate            double          0.05            5.00%           Annualized savings rate earned by savers for depositing VIGOR tokens into VIGOR Collateral. The earnings are paid to savers automatically throughout the day denominated in VIG into their vigfees. Savings rates are floating and change continuously over time proportional to borrow rates. Savings rates increase when VIGOR market price drops below $1. Savings rates decrease when VIGOR market price rises above $1. Savings rates decrease as more savers deposit VIGOR into savings.
    //0    user         l_svalueofinsx      stressed value of insurance up     double  100.00       $100.00         model suggested dollar value of the total insurance asset portfolio (ex the specified user) in an up market stress event.
    //0    user         svalueofinsx        stressed value of insurance down   double  100.00       $100.00         model suggested dollar value of the total insurance asset portfolio (ex the specified user) in a down market stress event.
    //1    globals      Solvency            Solvency (down markets) double          1.2             1.2             Represents the level of capital adequacy of the system to back the VIGOR borrows. It is a measure that indicates if capital existing in the system is above or below the capital requirement suggested by stress testing for downside market stress events. Borrow rates increase when solvency is below target. Borrow rates decrease when solvency is above target.
    //1    globals      l_solvency          Solvency (up markets)   double          3.5             3.5             Represents the level of capital adequacy of the system to back the crypto borrows. It is a measure that indicates if capital existing in the system is above or below the capital requirement suggested by stress testing for upside market stress events. Borrow rates increase when solvency is below target. Borrow rates decrease when solvency is above target.
    //1    user    two fields are both a type of collateral: collateral and l_debt Collateral  vector of assets and a single asset na  na  Depositing cryptos as collateral allows for borrowing VIGOR. Depositing VIGOR allows for borrowing cryptos. Depositing VIGOR will earn the savings rate. During any borrow, the user collateral is at risk of bailout if the feed value of the collateral drops below the value of debt. Users must maintain a VIG balance above zero in vigfees because VIG fees are automatically deducted on a continuous basis, otherwise the user suffers immediate bailout. A bailout is an event where user retains excess collateral but the debt and matching amount of collateral is deducted from the borrower and taken by the insurers. There is no liquidation fee or additional charges for bailout.
    //1    user         insurance           Insurance               vector<asset>   na              na              Cryptos and VIGOR are deposited into the insurance pool to earn VIG and to insure the system against both upside and downside market stress events. Insurers agree to accept their share of bailouts (automatically get assigned ownership of failed collateral and associated debt) according to their contribution to solvency (PCTS).
    //1    user    two fields are both a type of debt: debt and l_collateral   Debt    a single asset and vector of assets na  na  Cryptos and VIGOR borrowed. Users can borrow up to a maximum 1.1 collateral ratio (value of collateral/value of debt)
    //1    user         pcts                PCTS (down markets)     double          0.015           1.50%           Percent contribution to solvency for downside market stress events. It measures the extent to which a given insurer improves the system Solvency (down markets). This determines how much the insurer is compensated relative to other insurers. It also determines the share of bailouts that will be assigned to the insurer during downside market stress events.
    //1    user         l_pcts              PCTS (up markets)       double          0.015           1.50%           Percent contribution to solvency for upside market stress events. It measures the extent to which a given insurer improves the system Solvency (up markets). This determines how much the insurer is compensated relative to other insurers. It also determines the share of bailouts that will be assigned to the insurer during upside market stress events.
    //0    user         collateral          Crypto Collateral       vector<asset>   na              na              Cryptos (not VIGOR) deposited as collateral for borrowing VIGOR. Collateral is at risk of bailout if the feed value of the collateral drops below the value of debt, or if user runs out of VIG in their Crypto Collateral and therefore cannot pay the automatically collected VIG fees.
    //0    user         l_debt              VIGOR Collateral        asset(VIGOR)    na              na              VIGOR deposited to earn the savings rate and to be held as collateral for borrowing cryptos. During a borrow, collateral is at risk of bailout if the feed value of the collateral drops below the value of debt, or if user runs out of VIG in their vigfees and therefore cannot pay the automatically collected VIG fees.
    //0    user         debt                VIGOR Debt              asset(VIGOR)    na              na              VIGOR borrowed. Users can borrow up to a maximum 1.1 collateral ratio (value of collateral/value of debt)
    //0    user         l_collateral        Crypto Debt             vector<asset>   na              na              Cryptos borrowed (not VIGOR). Users can borrow up to a maximum 1.1 collateral ratio (value of collateral/value of debt)
    //0    user         prem                premiums paid (VIGOR loans)     double 50.0003  50.0003                 Running total of VIG premiums paid by this user for purchasing insurance for their VIGOR loans. It is paid from this user's vigfees to the lender/insurance pool. The payments are denominated in VIG and deducted automatically throughout the day from the user vigfees. Borrow rates are floating and change continuously over time based on system riskiness as measured by Solvency (down markets).
    //0    user         l_prem              premiums paid (Crypto loans)    double 50.0004  50.0004                 Running total of VIG premiums paid by this user for purchasing insurance for their Crypto loans (not VIGOR). It is paid from this user's vigfees to the lender/insurance pool. The payments are denominated in VIG and deducted automatically throughout the day from the user vigfees. Borrow rates are floating and change continuously over time based on system riskiness as measured by Solvency (up markets).
    //0    user         rewardlend          pct LP tokens vs total          double 0.051       5.1%                 // accumulated rewards paid for all reward types (eg rex, vote proxy, lptoken.defi, txn fees from vigorlending and curve.sx for USDT deposits), in dollars, for the period since last atimer rollover (for example 86400 sec)
    //0    user         txnvolume           total daily transactions        double 50.0001  $50.0001                total dollar amount of outbound transactions for the current day, timer is _g_atimer

    // nomenclature note:
    // this contract has two major features that are mirror images of each other:
    // 1 token repo: (borrow VIGOR against collateral (EOS tokens)
    // 2 token lend: (borrow EOS tokens against collateral (VIGOR tablecoin)
    // 'collateral' or 'debt' to one user could mean VIGOR, but to another user could mean EOS tokens
    // so prefix l_ is introduced to implement feature 2 by re-rusing the same code written initially for feature 1
    // any variable with prefix l_ is for 2 token lend. prefix l_ indicates 'reverse' (swap the word debt <-> collateral) in your mind
    // examples:
    // consider a token repo: when user locks EOS to borrow VIGOR, the VIGOR borrow is stored in debt and EOS is collateral
    // consider a token lend: when user locks VIGOR to borrow EOS, the VIGOR is stored in l_debt and EOS borrow is l_collateral
    // the variable 'debt' means user is borrowing VIGOR in a token repo
    // the variable "l_debt" means users is locking VIGOR as collateral in a token lend
    // the variable 'collateral' means user is locking EOS as collateral in a token repo
    // the variable "l_collateral" means user is borrowing EOS in a token lend

    auto primary_key() const { return usern.value; }
    uint64_t byreputation() const { return ((uint64_t)(10000 * this->getReputation())); }

    void setAnniv(eosio::time_point present_time, uint32_t anniv) {
      // convert present time to next anniversary in the future
      static const uint64_t now = present_time.sec_since_epoch();
      static const eosio::time_point_sec expirationdate = eosio::time_point_sec(now + anniv);
      reputation.setAnniv(expirationdate);
    }

    eosio::time_point getAnniv() const { return reputation.getAnniv(); }
    double getReputation() const { return reputation.getReputation(); }
    double getReputation_pct() const { return reputation.getReputation_pct(); }
    void setReputation(double c_score) { reputation.setReputation(c_score); }
    void setReputation_pct(double c_score) { reputation.setReputation_pct(c_score); }
    void reputationrotation() { return reputation.reputationrotation(); }
    void resetReputation() { reputation.resetReputation(); }

    void setVig_since_anniv(eosio::asset vig_since_anniv) { reputation.setVig_since_anniv(vig_since_anniv); }
    eosio::asset getVig_since_anniv() const { return reputation.getVig_since_anniv(); }

    void setRep_lag0(eosio::asset rep_lag0) { reputation.setRep_lag0(rep_lag0); }
    eosio::asset getRep_lag0() const { return reputation.getRep_lag0(); }
    eosio::asset getRep_lag1() const { return reputation.getRep_lag1(); }

    asset_container getInsurances() const { return asset_container(insurance); }
    asset_container getCollaterals() const { return asset_container(collateral); }
    asset_container getLCollaterals() const { return asset_container(l_collateral); }
    bool operator==(const user_s& other) const { return other.usern == usern; }
    bool operator!=(const user_s& other) const { return other.usern != usern; }

    bool isBorrower() const { return debt.amount > 0 || l_valueofcol > 0; }
    bool isEligible(eosio::name bailoutuser, eosio::name bailoutupuser, eosio::name finalreserve, eosio::name vigordacfund) const {
      if(usern == bailoutuser || usern == bailoutupuser)
        return false;
      return true;
    }

    EOSLIB_SERIALIZE(user_s, (usern)(debt)(reputation)(collateral)(insurance)(valueofcol)(valueofins)(tesprice)(earnrate)(pcts)(volcol)(txnvolume)(rewardsave)(prem)(svalueofinsx)(lastupdate)(l_debt)(l_collateral)(l_valueofcol)(l_tesprice)(l_pcts)(l_volcol)(savings)(rewardrexvot)(l_prem)(l_svalueofinsx)(vigfees)(rewardlend)(rewardlend2))
  };

  typedef eosio::multi_index<"user"_n, user_s,
                             eosio::indexed_by<"reputation"_n, eosio::const_mem_fun<user_s, uint64_t, &user_s::byreputation>>>
    user_t;
  user_t _u;
 public:
  TABLE globalstats {
    double solvency = 1.0;
    double valueofcol = 0.0;       // dollar value of collateral
    double valueofins = 0.0;       // dollar value of insurance
    double scale = 1.0;            // TES pricing model parameters are scaled to drive risk (solvency) to a target set by custodians.
    double svalueofcole = 0.0;     // model suggested dollar value of the sum of all insufficient collateral in a stressed market SUM_i [ min((1 - svalueofcoli ) * valueofcoli - debti,0) ]
    double svalueofins = 0.0;      // model suggested dollar value of the total insurance asset portfolio in a stress event. (1 - stressins ) * valueofins
    double volume = 0.0;           // volume, sum of all transactions since last full update (_g.lastupdate), in dollars
    double svalueofcoleavg = 0.0;  // model suggested dollar value of the sum of all insufficient collateral on average in down markets, expected loss
    double pcts = 0.0;             // total sum of user pcts (percent contribution to solvency)
    double savings = 0.0;          // total sum of user VIGOR in savings, units number of VIGOR
    double premiums = 0.0;         // total dollar amount of premiums all borrowers would pay in one year to insure their collateral
    double rm = 0.0;               // sum of weighted marginal contribution to risk (solvency)
    double earnrate = 0.0;         // annualized rate of return on total portfolio of insurance crypto assets, insuring for downside and upside price jumps
    double savingsrate = 0.0;
    eosio::time_point lastupdate = eosio::current_time_point();  // time of last full update
    eosio::asset fee = eosio::asset(0, VIG10_SYMBOL);            // VIG fees collected from VIGOR borrowers, for carrying debt for the time period since last transaction, and distributed
    std::vector<eosio::time_point> availability;

    eosio::asset totaldebt = eosio::asset(0, VIGOR_SYMBOL);  // VIGOR borrowed
   protected:
    asset_container_t insurance;
    asset_container_t collateral;

   public:
    double l_solvency = 1.0;
    double l_valueofcol = 0.0;                                 // dollar value of l_collateral
    double l_scale = 1.0;                                      // TES pricing model parameters are scaled to drive risk (solvency) to a target set by custodians.
    double l_svalueofcole = 0.0;                               // model suggested dollar value of the sum of all insufficient collateral in a stressed market SUM_i [ min((1 - svalueofcoli ) * valueofcoli - debti,0) ]
    double l_svalueofins = 0.0;                                // model suggested dollar value of the total insurance asset portfolio in a stress event. ( 1.0 + l_stressins ) * valueofins
    double l_svalueofcoleavg = 0.0;                            // model suggested dollar value of the sum of all insufficient collateral on average in up markets, expected loss
    double l_pcts = 0.0;                                       // sum of user l_pcts (percent contribution to l_solvency)
    double l_premiums = 0.0;                                   // total dollar amount of premiums all borrowers would pay in one year to insure their l_collateral
    double l_rm = 0.0;                                         // sum of weighted marginal contribution to risk (l_solvency)
    eosio::asset l_fee = eosio::asset(0, VIG10_SYMBOL);        // VIG fees collected and distributed from VIGOR borrowers, for carrying debt for the time period since last transaction
    eosio::asset l_totaldebt = eosio::asset(0, VIGOR_SYMBOL);  // VIGOR locked as collateral
    uint16_t step = 0;                                         // current step in doupdate. step 0 is clean, anything else means contract is updating at that step.
    uint64_t ac = 0;                                           // number of users
    double savingsscale = 1.0;                                 // scale factor applied to savingscut. when VIGOR feed price is below 1.0 then savingsscale is > 1.0 thus increasing the savings rate to incentivise hodling/locking VIGOR into savings (and vice versa)
    eosio::time_point kicktimer = eosio::current_time_point();
    eosio::name bailoutuser;
    eosio::name bailoutupuser;
    uint64_t bailoutid = 0;
    uint64_t rexproxy = 0;         // accumulated REX rewards and proxy voter rewards since last time distribution to users, units number of EOS * pow(10,4)
    uint64_t rexproxya = 0;        // accumulated EOS rewards from REX and proxy voter rewards since last time atimer rollover (for example 86400 sec) to users holding EOS, units number of EOS
    uint64_t rexproxyp = 0;        // last period accumulated EOS rewards from REX and proxy voter rewards, period defined by atimer, to users holdind EOS, units number of EOS
    uint64_t vigfees = 0;          // total sum number of VIG in all users vigfees
    double totalvalue = 0;         // total value of all users as of last full update (_g.lastupdate,) , in dollars, ins + col - debt
    eosio::time_point_sec atimer;  // timer for limiting a user total dollar amount of outbound transactions
    double lprewards = 0.0;        // accumulated dollar value of BOX, EOS, VIG rewards claimed from lppool.defi, lppool.defi, usnpool.defi since last time distribfee to users holding LP tokens lptoken.defi denominated as number of dollars
    double lprewardsa = 0.0;       // accumulated dollar value of BOX, EOS, VIG rewards claimed from lppool.defi, lppool.defi, usnpool.defi since last atimer rollover (for example 86400 sec) to users holding LP tokens lptoken.defi denominated as number of dollars    
    double lprewardsp = 0.0;       // last period accumulated dollar value of BOX, EOS, VIG rewards claimed from lppool.defi, lppool.defi, usnpool.defi, period defined by atimer, given to users holding LP tokens lptoken.defi denominated as number of dollars
    double totalrexvot = 0.0;      // total dollar value of EOS in global collateral, insurance, l_collateral, number of dollars, so 34.7 would mean 34.7 dollars
    double totrewlend = 0.0;       // total dollar value of LP tokens (lptoken.defi) in global collateral, insurance, l_collateral, number of dollars, so 34.7 would mean 34.7 dollars
    double txnfee = 0.0;           // accumulated transaction fees collected by vigordacfund in various tokens converted to dollars, units are dollars since last time distribfee
    double txnfeea = 0.0;          // accumulated transaction fees collected by vigordacfund in various tokens converted to dollars, units are dollars since last atimer rollover (for example 86400 sec)
    double txnfeep = 0.0;          // last period accumulated dollar value of transaction fees collected, units are dollars
    double ratesub = 0.0;          // accumulated subsidy, vigordacfund has been paying part of the borrow rates, _config.getSubsidy, units are dollars
    double liqcut = 0.0;           // accumulated liquidation penalties taken by vigordacfund, based on cut of the liquidation penalty _config.getLiqCut(), units is dollars
    double totrewlendsx = 0.0;     // total dollar value of USDT in global collateral, insurance, l_collateral, number of dollars, so 34.7 would mean 34.7 dollars
    double earnrater = 0.0;        // annualized rate of return on total portfolio of insurance and collateral crypto assets, from all reward types (eg rex proxy, LP tokens)
    double curvesxvol = 0.0;       // volume of transactions of SXE tokens on curve.sx, as of last time distribfee to users holding USDT tokens, units number of dollars
    double curvesxrew = 0.0;       // accumulated dollar value of transaction fees earned by deposits into curve.sx since last time distribfee to users holding USDT tokens denominated as number of dollars
    double curvesxrewa = 0.0;       // accumulated dollar value of transaction fees earned by deposits into curve.sx since last atimer rollover (for example 86400 sec) to users holding USDT tokens denominated as number of dollars
    double curvesxrewp = 0.0;       // last period accumulated dollar value of transaction fees earned by deposits into curve.sx, period defined by atimer, given to users holding USDT tokens denominated as number of dollars
   protected:
    asset_container_t l_collateral;

   public:
    asset_container getInsurances() const { return asset_container(insurance); }
    asset_container getCollaterals() const { return asset_container(collateral); }
    asset_container getLCollaterals() const { return asset_container(l_collateral); }

    EOSLIB_SERIALIZE(globalstats, (solvency)(valueofcol)(valueofins)(scale)(svalueofcole)(svalueofins)(volume)(svalueofcoleavg)(pcts)(savings)(premiums)(rm)(earnrate)(savingsrate)(lastupdate)(fee)(availability)(totaldebt)(insurance)(collateral)(l_solvency)(l_valueofcol)(l_scale)(l_svalueofcole)(l_svalueofins)(l_svalueofcoleavg)(l_pcts)(l_premiums)(l_rm)(l_fee)(l_totaldebt)(step)(ac)(savingsscale)(kicktimer)(bailoutuser)(bailoutupuser)(bailoutid)(rexproxy)(rexproxya)(rexproxyp)(vigfees)(totalvalue)(atimer)(lprewards)(lprewardsa)(lprewardsp)(totalrexvot)(totrewlend)(txnfee)(txnfeea)(txnfeep)(ratesub)(liqcut)(totrewlendsx)(earnrater)(curvesxvol)(curvesxrew)(curvesxrewa)(curvesxrewp)(l_collateral))
  };

  typedef eosio::singleton<"globalstats"_n, globalstats> globals;
  globals _globals;  //  Do not use this. Use _g instead.
  globalstats _g;

  asset_container getInsurances(user_s & user) const {
    return user.getInsurances().setGlobal([this]() { return _g.getInsurances(); });
  }
  asset_container getCollaterals(user_s & user) const {
    return user.getCollaterals().setGlobal([this]() { return _g.getCollaterals(); });
  }
  asset_container getLCollaterals(user_s & user) const {
    return user.getLCollaterals().setGlobal([this]() { return _g.getLCollaterals(); });
  }
 private:
 
  TABLE currency_stats {
    eosio::asset supply;
    eosio::asset max_supply;
    eosio::name issuer;
    uint64_t primary_key() const { return supply.symbol.code().raw(); }
  };
  typedef eosio::multi_index<"stat"_n, currency_stats> currency_statst;
  currency_statst _currency_stats;

  // user deposits stake to open account
  TABLE stakeacnt {
    eosio::name usern;
    eosio::asset stake = eosio::asset(0, VIG10_SYMBOL);
    eosio::time_point_sec unlocktime;
    auto primary_key() const { return usern.value; }
    //EOSLIB_SERIALIZE(stakeacnt, (usern)(stake)(unlocktime))
  };
  typedef eosio::multi_index<"stake"_n, stakeacnt> staked;
  staked _staked;

  // whitelist for max dollar limit of total account value
  TABLE acctsize {
    eosio::name usern;
    uint64_t limit;
    auto primary_key() const { return usern.value; }
    //EOSLIB_SERIALIZE(acctsize, (usern)(limit))
  };
  typedef eosio::multi_index<"acctsize"_n, acctsize> acctsize_t;
  acctsize_t _acctsize;

  TABLE log_s {
    uint64_t id;
    eosio::name usern;
    eosio::name function;
    std::string message;
    eosio::time_point timestamp;
    uint64_t bailoutid;
    uint64_t primary_key() const { return id; }
    uint64_t by_timestamp() const { return timestamp.sec_since_epoch(); }
    uint64_t by_usern() const { return usern.value; }
    uint64_t by_function() const { return function.value; }
    uint64_t by_bailoutid() const { return bailoutid; }
  };
  typedef eosio::multi_index<"log"_n, log_s,
                             eosio::indexed_by<"bytimestamp"_n, eosio::const_mem_fun<log_s, uint64_t, &log_s::by_timestamp>>,
                             eosio::indexed_by<"byusern"_n, eosio::const_mem_fun<log_s, uint64_t, &log_s::by_usern>>,
                             eosio::indexed_by<"byfunction"_n, eosio::const_mem_fun<log_s, uint64_t, &log_s::by_function>>,
                             eosio::indexed_by<"bybailoutid"_n, eosio::const_mem_fun<log_s, uint64_t, &log_s::by_bailoutid>>>
    log_t;
  log_t _log;

  TABLE bailout_s {
    uint64_t id;
    eosio::time_point timestamp;
    eosio::name usern;
    uint64_t bailoutid;
    eosio::name type;
    double pcts;
    asset_container_t debt;
    asset_container_t collateral;
    asset_container_t recap1;
    asset_container_t recap2;
    asset_container_t recap3;
    asset_container_t recap3b;
    asset_container_t blockedins;
    asset_container_t blockeddebt;

    uint64_t primary_key() const { return id; }
    uint64_t by_timestamp() const { return timestamp.sec_since_epoch(); }
    uint64_t by_usern() const { return usern.value; }
    uint64_t by_bailoutid() const { return bailoutid; }
    uint64_t by_type() const { return type.value; }
  };
  typedef eosio::multi_index<"bailout"_n, bailout_s,
                             eosio::indexed_by<"bytimestamp"_n, eosio::const_mem_fun<bailout_s, uint64_t, &bailout_s::by_timestamp>>,
                             eosio::indexed_by<"byusern"_n, eosio::const_mem_fun<bailout_s, uint64_t, &bailout_s::by_usern>>,
                             eosio::indexed_by<"bybailoutid"_n, eosio::const_mem_fun<bailout_s, uint64_t, &bailout_s::by_bailoutid>>,
                             eosio::indexed_by<"bytype"_n, eosio::const_mem_fun<bailout_s, uint64_t, &bailout_s::by_type>>>
    bailout_t;
  bailout_t _bailout;

  TABLE batchse {
    eosio::name usern;  //user batching, user at head of current batch
    uint64_t batchsize = 40;
    uint64_t stepdata;
    uint64_t stepdata2;
    double sumpcts;
    double excesspct;
    double l_pctl = 1.0;
    double l_rmliq = 0.0;
    bool lastflag;
    eosio::name buser;
    eosio::name bupuser;
    eosio::name lastinsurer;
    asset_container_t assetcont;
    eosio::asset asset;
  };

  typedef eosio::singleton<"batchse"_n, batchse> batche;
  batche _batche;  //  Do not use this. Use _b instead.
  batchse _b;
  // replica of the member table from dac token
  struct member {
    eosio::name sender;
    // agreed terms version
    uint64_t agreedtermsversion;
    uint64_t primary_key() const { return sender.value; }
    EOSLIB_SERIALIZE(member, (sender)(agreedtermsversion))
  };
  typedef eosio::multi_index<"members"_n, member> regmembers;

  // replica of the termsinfo table from dac token
  struct termsinfo {
    std::string terms;
    std::string hash;
    uint64_t version;
    termsinfo() : terms(""), hash(""), version(0) {}
    termsinfo(std::string _terms, std::string _hash, uint64_t _version) : terms(_terms), hash(_hash), version(_version) {}
    uint64_t primary_key() const { return version; }
    uint64_t by_latest_version() const { return UINT64_MAX - version; }
    EOSLIB_SERIALIZE(termsinfo, (terms)(hash)(version))
  };
  typedef eosio::multi_index<"memberterms"_n, termsinfo,
                             eosio::indexed_by<"bylatestver"_n,
                                               eosio::const_mem_fun<termsinfo, uint64_t, &termsinfo::by_latest_version>>>
    memterms;

  //From vigoraclehub, holds the time series of prices, returns, volatility and correlation
  struct statspre {
    uint32_t freq;
    eosio::time_point timestamp;
    std::deque<uint64_t> price;
    std::deque<int64_t> returns;
    std::map<eosio::symbol, int64_t> correlation_matrix;
    std::uint64_t vol = defaultVol;

    uint32_t primary_key() const {
      return freq;
    }
  };
  typedef eosio::multi_index<"tseries"_n, statspre> t_series;
  t_series _statstable;

  TABLE whitelist_s {
    eosio::symbol sym;
    eosio::name contract;
    eosio::name feed;
    bool assetin;           // false means do not allow assetin
    bool assetout;          // false means do not allow assetout
    uint8_t maxlends;       // maximum percentage allowed to lend as a percentage of lending pool. 5 means 5%
    eosio::asset lendable;  // amount of token available to be lent. units: number of tokens, (ignores maxlends)
    double lendablepct;     // lendable amount of token as a percentage of total lending pool. percentage of this token lendable value relative to total value of lending pool; it's weight in the lending pool, (ignores maxlends)
    double lentpct;         // a measure of scarcity for borrowing. percentage of this token lent relative to the lendable amount subject to maxlends; lent utilization. 100% means that the lent amount has reached maxlends
                            //Borrow rates: lendpct is used to determine total portfolio scarcity for each user portfolio of borrowed tokens (user.l_collateral). This liquidity risk type informs the pricing model.
                            //Lending rewards: are influenced by scarcity of lendable tokens in the lending pool, "percent contribution to liquidity" to encourage deposits of scarce tokens.
                            //The formulation uses the fields in the _whitelist table to arrive at a portfolio scarcity measure for lender portfolios (user.insurance). earnrate formula also reflects this.
    uint64_t primary_key() const { return sym.raw(); }
  };
  typedef eosio::multi_index<"whitelist"_n, whitelist_s> whitelist_t;
  whitelist_t _whitelist;

  TABLE market_s {
    eosio::symbol sym;
    statspre marketdata;
    uint64_t primary_key() const { return sym.raw(); }
  };
  typedef eosio::multi_index<"market"_n, market_s> market_t;
  market_t _market;

  struct [[eosio::table, eosio::contract("eosio.system")]] rex_balance {
    uint8_t version = 0;
    eosio::name owner;
    eosio::asset vote_stake;
    eosio::asset rex_balance;
    int64_t matured_rex = 0;
    std::deque<std::pair<eosio::time_point_sec, int64_t>> rex_maturities;  /// REX daily maturity buckets

    uint64_t primary_key() const { return owner.value; }
  };

  typedef eosio::multi_index<"rexbal"_n, rex_balance> rex_balance_table;
  rex_balance_table _rex_balance_table;

  struct [[eosio::table, eosio::contract("eosio.system")]] rex_pool {
    uint8_t version = 0;
    eosio::asset total_lent;
    eosio::asset total_unlent;
    eosio::asset total_rent;
    eosio::asset total_lendable;
    eosio::asset total_rex;
    eosio::asset namebid_proceeds;
    uint64_t loan_num = 0;

    uint64_t primary_key() const { return 0; }
  };

  typedef eosio::multi_index<"rexpool"_n, rex_pool> rex_pool_table;
  rex_pool_table _rex_pool_table;

  struct [[eosio::table]] account {
    eosio:: asset    balance;

    uint64_t primary_key()const { return balance.symbol.code().raw(); }
  };

  typedef eosio::multi_index< "accounts"_n, account > accounts;
  accounts _accounts;

  void risk();
  void l_risk();
  void stresscol(const user_s& user);
  void l_stresscol(const user_s& user);
  void stressins();
  void l_stressins();
  void stressinsx(const user_s& user);
  double portfolioVariance(asset_container_t & container, double valueOf);
  double portfolioVariancex(asset_container_t & container, double valueOf, const user_s& user, double& user_valueofins_adj);
  void update(const user_s& user);
  void updateglobal();
  void distribfee();
  void collectfee();
  void recap(const user_s& user, const user_s& insurerUser, double insurerpcts, double& sumpcts, bool lastInsurer);
  void recapup(const user_s& user, const user_s& insurerUser, double insurerlpcts, double& sumpcts, bool lastInsurer);
  void pricing(const user_s& user);
  void l_pricing(const user_s& user);
  void performance(const user_s& user, const double& vigvol, const double& vigprice);
  void performanceglobal();
  void pcts(const user_s& user);
  void l_pcts(const user_s& user);
  void RM(const user_s& user);
  void l_RM(const user_s& user);
  void reserve();
  void paybacktok(const user_s& user, eosio::asset assetin);
  double approx(double t);
  double icdf(double p);
  template <typename... T>
  void schaction(const eosio::name& tag, const eosio::name& action_name, const std::tuple<T...> data, const uint32_t& delay_sec, const uint32_t& expiry_sec, const uint64_t& repeat);
  void tracetable(const char* fname, int lineno, const eosio::name& usern, const uint64_t& bailoutid, const std::string& message);
  eosio::asset b_reputation(const user_s& user, uint64_t viga, uint64_t l_viga);
  eosio::asset i_reputation(eosio::asset viga);
  void reputationrank();
  void exassetout(const eosio::name& usern, const eosio::asset& assetout, const std::string& memo, const bool& exec, const bool partialupdate, const uint64_t& step, double& totalval, double& totalvalbefore);
  double l_pctl(const user_s& user);
  eosio::symbol symbol_swap(const eosio::symbol& s);
  eosio::asset asset_swap(const eosio::asset& a);
  void takefee(eosio::asset fee);
  void sellrex(eosio::asset asset);
  void payminer(const eosio::name& miner, uint64_t fee);

  struct bname_s {
    eosio::name usern;
  };



  //   #if TESTMODE
  template <typename T>
  void cleanTable(eosio::name code, uint64_t account, const uint32_t batchSize) {
    T db(code, account);
    uint32_t counter = 0;
    auto itr = db.begin();
    while(itr != db.end() && counter++ < batchSize) {
      itr = db.erase(itr);
    }
  }

 public:
  TABLE config {
  // alphatest      rarity for the stress test, defined as an event with probability (1-alphatest)
  // soltarget      solvency target, when solvency is above or below this target then price inputs are scaled to dampen borrowing and encourage lender/insurers. if solvency is below target then more of the risk budget is being used than is recommended by the solvency capital requirement
  // lsoltarget     l_solvency target. same as solvency target except that solvency is in regards to downside market stress and l_solvency is in regards to upside market stress
  // maxtesprice    upper limit on the tesprice aka borrow rate, 0.5 would be a 50% annualized maximum borrow rate
  // mintesprice    lower limit on the tesprice aka borrow rate, 0.005 would be a 0.5% annualized minimum borrow rate
  // calibrate      a scale factor on tesprice. tesprice is based on a one year tenor as an initial condition, solvency may come to rest above or below target, calibrate can adjust that initial condition degree of freedom
  // maxtesscale    upper limit on the price adjustment factor when solvency is below target. 2.0 means that the price inputs can be scaled but no more than 2x
  // mintesscale    lower limit on the price adjustment factor when solvency is above target. 0.1 means that the price inputs can be scaled but no smaller than a factor of 0.1
  // reservecut     percentage of fees collected paid into reserve
  // savingscut     percentage of fees collected paid to savers
  // maxlends       max percentage of insurance tokens allowable to lend out
  // freezelevel    lock down specific actions in the contract. 0 unfreeze, 1 freeze assetout, 2 freeze assetin, 3 freeze assetout & assetin, 4 freeze assetout & assetin & doupdate
  // assetouttime   delay time in seconds for users calling assetin
  // initialvig     not used
  // viglifeline    not used
  // vigordaccut    cut of the vig fees taken by vigor dac
  // newacctlim     number of new accounts limit: can create this many new accounts: newacctlim per newacctsec
  // newacctsec     number of seconds for limiting new account: can create this many new accounts: newacctlim per newacctsec
  // reqstake       dollar amount of VIG tokens required to stake to open an account when account limit is reached per day
  // staketime      seconds to lock the user deposited new account stake
  // repanniv       anniverary for reputation score to be rotated, for example every thirty days (stated in seconds)
  // maxdisc        maximum discount off the borrow rate. a 10% discount would reduce a borrow rate of 20% to 18% (.2*(1-.1))
  // execType       exectution type, 1=deferred 2=mining txn 3=inline
  // minebuffer     amount of time miners have to mine before jobs expire
  // gasfee         not used
  // kickseconds    0 = turn off kick user. [Units is seconds]. The lowest ranked user is kicked every kickseconds period. Kick means: tokens returned to user, user liquidated, user account deleted
  // initmaxsize    total account value cannot exceed this dollar amount
  // assetintime    delay time in seconds for users calling assetin
  // debtceiling    upper limit on how many VIGOR can be issued from supply.
  // logcount       how many log messages to store in the log table
  // gatekeeper     0 = no gatekeeper, 1 = restrict to members 2 = restrict to candidates having one or more votes, 3 = restrict to custodians
  // liquidate      0 = turn off self liquidation, 1 = turn on
  // accountslim    uppler limit on the number of accounts that can exist in the user table
  // mincollat      not used
  // rexswitch      0 = turn off REX; any non-zero value is turn on REX
  // dataa          for a given user, this is limit for the total dollar amount of outbound transactions per day
  // datab          for a given user, this is limit for the dollar amount of a single outbound transaction 
  // datac          calibration paramter, premultiplier to tenor in the pricing model
  // bailoutcr      The CR where bailout and bailoutup is triggered, also the percentage of additional collateral distributed to lenders from the borrower. example 100 is 100% CR with 0% collateral bonus while 110 is 110% CR with 10% collateral bonus.
  // ratesub        vigordacfund paying for this percentage of the borrow rate, and makes up the differnce collecting transaction fees and liquidiation penalties, 0.25 means dac is paying 25% of the rate
  // liqcut         percentage of the liquidation penalty taken by vigordacfund, for example if bailoutCR is 120% and liqcut is 0.1 then dac will take 10% of the 20% which is 2% liquidation penalty fee
  // txnfee         transaction fee in bps. 10 would mean that 0.1% would be taken as a fee for a given transaction size. fees taken from collaterl, insurance, l_debt
  // minerfee       fee paid to miners calling rewardclaim and rewardswap. units are VIG4 precision. so 1 would mean 0.0001 VIG
  // lpsxcut        percentage of the transaction fees given to depositors of USDT
  // minc           limit users from taking a new borrow such that the ratio of collateral to debt is above bailoutCR and bailoutupCR, for example 0.05 is bailoutCR + 5%
  // tickfee        fee paid to miners calling tick action. units are VIG4 precision. so 1 would mean 0.0001 VIG
#define SEQUENCE                                                                                         \
  FIELD(alphatest, AlphaTest, FIELD_DATA_DOUBLE_TO_INT(uint16_t, 1000, 500, 999, 0.9))                   \
  FIELD(soltarget, SolvencyTarget, FIELD_DATA_DOUBLE_TO_INT(uint16_t, 10, 1, 1000, 1.0))                 \
  FIELD(lsoltarget, LSolvencyTarget, FIELD_DATA_DOUBLE_TO_INT(uint16_t, 10, 1, 1000, 2.5))               \
  FIELD(maxtesprice, MaxTesPrice, FIELD_DATA_DOUBLE_TO_INT(uint16_t, 10000, 1, 10000, 0.5))              \
  FIELD(mintesprice, MinTesPrice, FIELD_DATA_DOUBLE_TO_INT(uint16_t, 10000, 1, 10000, 0.005))            \
  FIELD(calibrate, Calibrate, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 10, 1, 100, 1.0))                        \
  FIELD(maxtesscale, MaxTesScale, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 10, 1, 100, 2.0))                    \
  FIELD(mintesscale, MinTesScale, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 10, 1, 100, 0.1))                    \
  FIELD(reservecut, ReserveCut, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 30, 0.2))                      \
  FIELD(savingscut, SavingsCut, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 30, 0.1))                      \
  FIELD(maxlends, MaxLends, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 100, 0.3))                         \
  FIELD(freezelevel, FreezeLevel, FIELD_DATA_NUMBER(uint16_t, 0, 10, 0))                                 \
  FIELD(assetouttime, AssetOutTime, FIELD_DATA_NUMBER(uint32_t, 0, 86400, 35))                           \
  FIELD(initialvig, InitialVig, FIELD_DATA_NUMBER(uint16_t, 0, 65535, 0))                                \
  FIELD(viglifeline, vigLifeLine, FIELD_DATA_NUMBER(uint8_t, 1, 90, 3))                                  \
  FIELD(vigordaccut, VigorDacCut, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 50, 0.1))                    \
  FIELD(newacctlim, NewAcctLim, FIELD_DATA_NUMBER(uint8_t, 0, 255, 10))                                  \
  FIELD(newacctsec, NewAcctSec, FIELD_DATA_NUMBER(uint32_t, 60, 90 * 60 * 60 * 24, 1 * 60 * 60 * 24))    \
  FIELD(reqstake, ReqStake, FIELD_DATA_NUMBER(uint16_t, 0, 1000, 100))                                   \
  FIELD(staketime, StakeTime, FIELD_DATA_NUMBER(uint32_t, 0, 2592000, 3 * 86400))                        \
  FIELD(repanniv, RepAnniv, FIELD_DATA_NUMBER(uint32_t, 60, 90 * 60 * 60 * 24, 30 * 60 * 60 * 24))       \
  FIELD(maxdisc, MaxDisc, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 100, 0.25))                          \
  FIELD(exectype, ExecType, FIELD_DATA_NUMBER(uint8_t, 1, 4, 2))                                         \
  FIELD(minebuffer, MineBuffer, FIELD_DATA_NUMBER(uint32_t, 0, 30 * 60 * 60 * 24, 30))                   \
  FIELD(gasfee, GasFee, FIELD_DATA_NUMBER(uint64_t, 1, UINT64_MAX, 1))                                   \
  FIELD(kickseconds, KickSeconds, FIELD_DATA_NUMBER(uint32_t, 0, 360 * 60 * 60 * 24, 30 * 60 * 60 * 24)) \
  FIELD(initmaxsize, InitMaxSize, FIELD_DATA_NUMBER(uint32_t, 100, 10000000, 50000))                     \
  FIELD(assetintime, AssetInTime, FIELD_DATA_NUMBER(uint32_t, 0, 86400, 35))                             \
  FIELD(debtceiling, DebtCeiling, FIELD_DATA_NUMBER(uint64_t, 1, 100000000000, 250000))                  \
  FIELD(logcount, LogCount, FIELD_DATA_NUMBER(uint64_t, 0, 10000000, 3000))                              \
  FIELD(gatekeeper, GateKeeper, FIELD_DATA_NUMBER(uint16_t, 0, 6, 3))                                    \
  FIELD(liquidate, Liquidate, FIELD_DATA_NUMBER(uint16_t, 0, 1, 0))                                      \
  FIELD(accountslim, AccountsLim, FIELD_DATA_NUMBER(uint32_t, 1, UINT32_MAX, 100))                       \
  FIELD(mincollat, MinCollat, FIELD_DATA_DOUBLE_TO_INT(uint64_t, 100, 0, UINT64_MAX, 1.11))              \
  FIELD(rexswitch, RexSwitch, FIELD_DATA_NUMBER(uint8_t, 0, 255, 1))                                     \
  FIELD(dataa, DataA, FIELD_DATA_NUMBER(uint64_t, 0, UINT64_MAX, 10000))                                 \
  FIELD(datab, DataB, FIELD_DATA_NUMBER(uint64_t, 0, UINT64_MAX, 1000))                                  \
  FIELD(datac, DataC, FIELD_DATA_DOUBLE_TO_INT(uint64_t, 100, 0, 100, 0.7))                              \
  FIELD(datad, DataD, FIELD_DATA_NAME(eosio::name, "extrafield11"))                                      \
  FIELD(proxycontr, ProxyContr, FIELD_DATA_NAME(eosio::name, "vigorrewards"))                            \
  FIELD(proxypay, ProxyPay, FIELD_DATA_NAME(eosio::name, "genereospool"))                                \
  FIELD(dactoken, DacToken, FIELD_DATA_NAME(eosio::name, "dactoken1111"))                                \
  FIELD(oraclehub, OracleHub, FIELD_DATA_NAME(eosio::name, "vigoraclehub"))                              \
  FIELD(daccustodian, DacCustodian, FIELD_DATA_NAME(eosio::name, "daccustodia1"))                        \
  FIELD(vigordacfund, VigorDacFund, FIELD_DATA_NAME(eosio::name, "vigordacfund"))                        \
  FIELD(finalreserve, FinalReserve, FIELD_DATA_NAME(eosio::name, "finalreserve"))                        \
  FIELD(bailoutcr, BailoutCR, FIELD_DATA_DOUBLE_TO_INT(uint32_t, 1, 100, 150, 100))                      \
  FIELD(bailoutupcr, BailoutupCR, FIELD_DATA_DOUBLE_TO_INT(uint32_t, 1, 100, 150, 100))                  \
  FIELD(ratesub, RateSub, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 100, 0.0))                           \
  FIELD(liqcut, LiqCut, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 100, 0.5))                             \
  FIELD(txnfee, TxnFee, FIELD_DATA_NUMBER(uint64_t, 1, UINT64_MAX, 1))                                   \
  FIELD(minerfee, MinerFee, FIELD_DATA_NUMBER(uint64_t, 1, UINT64_MAX, 1))                               \
  FIELD(lpsxcut, LpsxCut, FIELD_DATA_DOUBLE_TO_INT(uint8_t, 100, 0, 100, 0.1))                           \
  FIELD(minc, MinC, FIELD_DATA_DOUBLE_TO_INT(uint64_t, 100, 0, UINT64_MAX, 0.05))                        \
  FIELD(tickfee, TickFee, FIELD_DATA_NUMBER(uint64_t, 1, UINT64_MAX, 1))                                 \

  TABLE_HANDLER(config, SEQUENCE)};

  typedef eosio::singleton<eosio::name("config"), config> config_singleton;
  config_singleton _configTable;
  config _config;

  bool clearSingletons = false;
  const char*  errormsg_missinguser = "User is missing from the user table, openaccount first.";
  vigorlending(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds);
  ~vigorlending();
  ACTION openaccount(const eosio::name& owner);
  ACTION acctstake(const eosio::name& owner);
  ACTION deleteacnt(const eosio::name& owner);
  ACTION dodeleteacnt(const eosio::name& owner);
  ACTION doupdate();
  ACTION predoupdate(const uint64_t& step);
  ACTION assetout(const eosio::name& usern, const eosio::asset& assetout, const std::string& memo);
  EVENT_HANDLER("*::transfer")
  assetin(const eosio::name& from, const eosio::name& to, const eosio::asset& assetin, const std::string& memo);
  ACTION doassetout(const eosio::name& usern, const eosio::asset& assetout, const std::string& memo, const bool& exec, const bool partialupdate, const uint64_t& step);
  ACTION configure(eosio::name key, std::string value);
  ACTION freezelevel(std::string value);
  ACTION log(const std::string& message) { eosio::require_auth(_self); }
  ACTION kick(const eosio::name& usern, const uint32_t& delay_sec);
  ACTION setacctsize(const eosio::name& usern, uint64_t& limit);
  ACTION tick(const eosio::name& miner);
  ACTION bailout(const eosio::name& usern);
  ACTION bailoutup(const eosio::name& usern);
  ACTION returncol(const eosio::name& usern);
  ACTION returnins(const eosio::name& usern);
  ACTION liquidate(const eosio::name& usern);
  ACTION liquidateup(const eosio::name& usern);
  ACTION whitelist(eosio::symbol sym, eosio::name contract, eosio::name feed, bool assetin, bool assetout, uint8_t maxlends);
  ACTION unwhitelist(eosio::symbol sym);
  ACTION cleanbailout(const uint32_t batchSize);
  ACTION clearconfig();
  ACTION setconfig(const config& config);
  ACTION rewardswap(const eosio::name& miner);
  ACTION clearglobal();
  ACTION setglobal(const globalstats& globalstats);
  ACTION rewardclaim(const eosio::name& miner, eosio::name contract);
  ACTION sweep();

#if defined(DEBUG)
  ACTION dbgresetall(const uint32_t numrows);
  ACTION dbgclrqueue(const uint32_t numrows);
  ACTION dbggivevig(const uint64_t quantity, const eosio::name usrn, const std::string memo);
#endif
  };
