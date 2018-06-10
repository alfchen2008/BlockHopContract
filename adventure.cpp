#include <eosiolib/eosio.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/dispatcher.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/core_symbol.hpp>
#include <stdio.h>

using namespace eosio;
using std::string;

  
class adventure : public contract {
  public:
    adventure( account_name  code):
    contract(code),  advdata(code, code), pricedata(code, code), _mycontract(code){
    }
 
    //@abi action
    void setprice(asset token, uint64_t price){
        print("setprice: ", token, ", ", price, "\n");
        require_auth(_mycontract);

        auto itr = pricedata.find(token.symbol);
        if(itr == pricedata.end()){
            pricedata.emplace(_mycontract, [&]( auto & e ){
                e.tokensym = token.symbol;
                e.price = price;
            });
        }else{
            pricedata.modify( itr, 0, [&]( auto & e ){
                e.price = price;
            });
        }
    }

    void exchange(account_name sender, asset systoken, string customtoken){
        bool invalid = true;
        asset tkntransfer;

        print("exchange ", sender, " ", systoken, " ", customtoken, "\n");

        symbol_name sym = string_to_symbol(4, customtoken.c_str());
        auto itr = pricedata.find(sym);
        if(itr != pricedata.end() && itr->price > 0){
            int64_t amount = systoken.amount * 10000 / itr->price;//SYS precesion is 4 by default
            tkntransfer = asset(amount, sym);
            invalid = false;
        }

        if(!invalid){
            print("send ", tkntransfer, " to ", sender, "\n");
            currency::inline_transfer(_mycontract, sender, extended_asset(tkntransfer, _mycontract), "sendtkn");
        }else{
            print("send ", systoken, " back to ", sender, "\n");
            currency::inline_transfer(_mycontract, sender, extended_asset(systoken, _mycontract), "sendback");
        }
    }

    void explore( account_name user , asset res, string memo) {
        print( "explore: ", name{user}, " with ", res, " and ", memo , "\n");
        require_auth( user );

        uint64_t idx = atoi(memo.c_str());
        //checksum256 cksum;    
        //sha256(const_cast<char*>(memo.c_str()), memo.length(), &cksum);
        //checksum256toUint128(cksum, lidx);    

        auto itr = advdata.find(idx);
        if(itr == advdata.end()){
            advdata.emplace(_mycontract, [&]( auto& d ){
                d.index = idx;
                d.player = user;
                d.resource = res;
                d.exploreblock = tapos_block_num();
                //todo: generate explore result according to exploreblock, num and predefined rules
                //      and send different tokens back to user
            });
        }else{
            advdata.modify( itr, 0, [&]( auto& d ) {
                eosio_assert(d.player == user, "different user, unexpected!!");
                d.resource += res;
            });
        }
    }

    //@abi table
    struct advdata{
        uint64_t        index;
        account_name     player;
        asset            resource;
        uint64_t         exploreblock;
        uint64_t primary_key() const{ return index;}
        EOSLIB_SERIALIZE( advdata, (index)(player)(resource)(exploreblock) )
    };

    typedef multi_index< N(advdata), advdata > advdata_index;

    //@abi table
    struct pricedata{
        symbol_name     tokensym;
        uint64_t        price;
        uint64_t primary_key() const{ return tokensym;}
        EOSLIB_SERIALIZE( pricedata, (tokensym)(price) )        
    };

    typedef multi_index< N(pricedata), pricedata > pricedata_index;
private:

    advdata_index advdata;
    pricedata_index pricedata;
    account_name _mycontract;
};


struct setpricedata{
    asset       token;
    uint64_t    price;
};

#ifdef ABIGEN
EOSIO_ABI( adventure, (setprice) )
#else
extern "C" {

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t act ) {
      print( "adventure: ", receiver ,"->", code, "->", act, "->", N(transfer), "\n" );

      adventure theadventure(receiver);
      
      if( act == N(transfer)) {
        //print("transfer\n");
        currency::transfer t = unpack_action_data<currency::transfer>();
        
        if(receiver == t.from) 
            print("SEND: ", t.from, "->", t.to, ", ", t.quantity,"\n");
        else
            print("RECV: ", t.from, "->", t.to, ", ", t.quantity,"\n");          

        //print("SYMBOL: ", symbol_name(t.quantity.symbol), ", ", S(4,FOD), "\n");
        if(t.to == receiver){
            if(symbol_name(t.quantity.symbol) == S(4,FOD)){                
                theadventure.explore( t.from , t.quantity, t.memo);                                
            }else if(symbol_name(t.quantity.symbol) == CORE_SYMBOL){                
                theadventure.exchange(t.from, t.quantity, t.memo);                
            }
        }        

        return;
      }else if( act == N(setprice)) {
          setpricedata d = unpack_action_data<setpricedata>();
          theadventure.setprice(d.token, d.price);
      }

    }

 } // extern "C"
#endif