#pragma once

#include <eosio/time.hpp>

#define VIG eosio::asset(0, eosio::symbol("VIG", 10))
#define NEG eosio::asset(-1, eosio::symbol("VIG", 10))

struct reputation_s {
private:
    double reputation; //Reputation is an average of the past 3 months VIG rate: average of rep_lag0, rep_lag1, rep_lag2
    double reputation_pct; // user reputation ranking as a percentile, example: 0.9 would mean top 90th percentile; a score better than 90% of all users. reputation_pct ranks all users against each other.
    eosio::asset vig_since_anniv; // running total for use in finding the VIG rate per day
    eosio::asset rep_lag0; //Current month VIG rate per day
    eosio::asset rep_lag1; //Last month VIG rate per day
    eosio::asset rep_lag2; //Two months ago VIG rate per day
    eosio::time_point anniv; //Reoccuring time in the future when rotation is done: rep_lag0 stored as rep_lag1. rep_lag1 stored as rep_lag2. anniv is a configurable feild.
/*
Reputation_pct: user rank between 0 and 100% compared to all users.
Good reputation -> deposit more into lending pool or take more debt than everyone else
It's a race to receive more VIG or pay more VIG over time, for example:
Bob: 5 VIG per day -> he is carrying 50% of total debt of the system and total VIG paid by everyone is 100 VIG over 10 days (50% * 100/10 = 5)
Sally: 10 VIG per day -> she has received 100 VIG on her deposit into lending over 10 days (100/10 = 10)
User "VIG per day" changes in real time, stored as rep_lag0. At each 30 day anniversary that value is stored as a historic measure of that past month into rep_lag1 (and rep_lag1 to rep_lag2). 
Reputation is an average of the past 3 months VIG rate per day. reputation_pct ranks all users against each other.
*/
public:
    // ctor
    reputation_s():rep_lag1(NEG), rep_lag2(NEG), reputation(0), reputation_pct(0), vig_since_anniv(VIG), rep_lag0(VIG), anniv(){}

    eosio::time_point getAnniv() const { return anniv; }
    void setAnniv( eosio::time_point current_time){
        anniv = current_time;
    }
    
    eosio::asset getVig_since_anniv() const { return vig_since_anniv; }
    void setVig_since_anniv(eosio::asset _vig_since_anniv){
        vig_since_anniv = _vig_since_anniv;
    }

    eosio::asset getRep_lag0() const { return rep_lag0; }
    void setRep_lag0(eosio::asset _rep_lag0){
        rep_lag0 = _rep_lag0;
        reputation = rep_lag0.amount / std::pow (10,10);
        if (rep_lag1.amount != -1)
            reputation += rep_lag1.amount / std::pow (10,10);
        if (rep_lag2.amount != -1)
            reputation += rep_lag2.amount / std::pow (10,10);
        reputation /= 3.0;
    }

    eosio::asset getRep_lag1() const { return rep_lag1; }

    double getReputation() const { return reputation; }
    void setReputation(double c_score){
        reputation = c_score;
    }

    // reputation reseting, when a bailout happens
    void resetReputation(){
        setVig_since_anniv(VIG);
        setRep_lag0(VIG);
    }

    double getReputation_pct() const { return reputation_pct; }
    void setReputation_pct(double c_score){
        reputation_pct = c_score;
    }

    // reputation reseting, when a bailout happens
    void resetReputation_pct(){
        reputation_pct = 0.0;
    }

    // reputation monthly roatation
    void reputationrotation();
    EOSLIB_SERIALIZE(reputation_s, (reputation)(reputation_pct)(vig_since_anniv)(rep_lag0)(rep_lag1)(rep_lag2)(anniv))
};

void reputation_s::reputationrotation(){

    rep_lag2 = rep_lag1;
    rep_lag1 = rep_lag0;
    setRep_lag0(VIG);
}