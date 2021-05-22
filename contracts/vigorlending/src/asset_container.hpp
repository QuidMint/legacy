#pragma once

#include<vector>
#include<eosio/asset.hpp>

typedef std::vector<eosio::asset> asset_container_t;

class asset_container {
    typedef asset_container_t::iterator asset_iterator_t;
    typedef asset_container_t::const_iterator asset_const_iterator_t;
    
    asset_container_t &container;
    std::function<asset_container ()> global;

    asset_container &_internal_addAsset(asset_container_t::iterator iter, const eosio::asset &asset) {
        if(asset.amount <= 0)
            return *this;
        
        if(iter == container.end())
            container.emplace_back(asset);
        else
            *iter += asset;

        if(global)
            global() += asset;
        
        return *this;
    }

    asset_container &_internal_subAsset(asset_container_t::iterator iter, const eosio::asset &asset) {
        if(iter == container.end())
            return *this;

        if(iter->amount == asset.amount)
            container.erase(iter);
        else
            *iter -= asset;

        if(global)
            global() -= asset;
        
        return *this;
    }

    /*
     * hasAsset
     * Returns true if an asset with the 'symbol' exists in the container.
     * 
     * symbol: The asset the contains the symbol to search for
     * iter: Output parameter to return the iterator to the found asset or container.end() if not found
     */
    bool _internal_hasAsset(const eosio::symbol &symbol, asset_iterator_t &iter) const {
        iter = std::find_if(container.begin(), container.end(), [&] ( const auto &item ) { return  item.symbol == symbol; } );
        return iter != container.end();
    }
    asset_iterator_t _internal_getAsset(const eosio::symbol &symbol) const {
        return std::find_if(container.begin(), container.end(), [&] ( const auto &item ) { return  item.symbol == symbol; } );
    }

public:
    asset_container(const asset_container_t &c)
        : container (const_cast<asset_container_t &>(c))
        {  };

    asset_container_t *operator -> () { return &container; }
    asset_container_t &operator * () { return container; }

    asset_container &operator += (const eosio::asset &asset) {
        return _internal_addAsset(_internal_getAsset(asset.symbol), asset);
    }
    asset_container &operator -= (const eosio::asset &asset) {
        return _internal_subAsset(_internal_getAsset(asset.symbol), asset);
    }

    template<typename F>
    asset_container &setGlobal(F g) {
        global = g;
        return *this;
    }

    //  hasAsset
    bool hasAsset(const eosio::symbol &symbol) const {
        return std::find_if(container.begin(), container.end(), [&] ( const auto &item ) { return  item.symbol == symbol; } ) != container.end();
    }
    bool hasAsset(const eosio::asset &asset) const {
        return hasAsset(asset.symbol);
    }
    bool hasAsset(const eosio::asset &asset, eosio::asset &assetToGet) const {
        asset_iterator_t iter;
        bool result = _internal_hasAsset(asset.symbol, iter);
        if(result)
            assetToGet = *iter;
        return result;
    }

    std::string to_string() {
        std::string message;
        for ( auto i : container)
            message.append(std::string(" " + i.to_string()));
        return message;
    }

    //  getAsset
    eosio::asset &getAsset(const eosio::asset &asset) const {
        return *_internal_getAsset(asset.symbol);
    }
    eosio::asset getAsset(const eosio::symbol &symbol) const {
        return *_internal_getAsset(symbol);
    }

    // isEmpty
    bool isEmpty(){ return container.empty(); }

    asset_container_t getContainer(){ return container; }
    
    //  Atomic move
    void move(const eosio::asset &asset, asset_container destination) {
        *this -= asset;
        destination += asset;
    }
};
