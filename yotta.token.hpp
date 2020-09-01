#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>
#include <string>
using namespace eosio;
using std::string;

/**
 * yotta.token contract defines the structures and actions that allow users to create, issue, and manage
 * tokens on eosio based blockchains.
 */
class [[eosio::contract("yotta.token")]] yottatoken : public contract {
   public:
      using contract::contract;

      //static constexpr symbol token_symbol = symbol(symbol_code(TOKEN_SYMBOL), 4);

      /**
       *  This action set account for uniqueness check contract deployment.
       *
       * @param account - account for contract deployment.
       */
      [[eosio::action]]
      void setunicheck( const name& account );

      /**
       * Allows `issuer` account to create a token in supply of `maximum_supply`. If validation is successful a new entry in statstable for token symbol scope gets created.
       *
       * @param issuer - the account that creates the token,
       * @param maximum_supply - the maximum supply set for the token created,
       * @param token_name - 通证名称,
       * @param memo - 通证描述.
       *
       * @pre Token symbol has to be valid,
       * @pre Token symbol must not be already created,
       * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
       * @pre Maximum supply must be positive;
       */
      [[eosio::action]]
      void create( const name&   issuer,
                   const asset&  maximum_supply,
                   const string& token_name,
                   const string& memo );

      /**
       *  This action issues to `to` account a `quantity` of tokens.
       *
       * @param to - the account to issue tokens to, it must be the same as the issuer,
       * @param quntity - the amount of tokens to be issued,
       * @param memo - the memo string that accompanies the token issue transaction.
       */
      [[eosio::action]]
      void issue( const name& to, const asset& quantity, const string& memo );

      /**
       *  This action set exchanging time.
       *
       * @param time - the exchanging time,
       * @param value - in order to get the symbol of currency.
       */
      [[eosio::action]]
      void setextime( uint64_t time, const asset& value );
      
      /**
       * Create an account.
       *
       * @param owner - which account will be created,
       * @param value - in order to get the symbol of currency,
       * @param ram_payer - which account create.
       */
      [[eosio::action]]
      void open( const name&    owner,
                 const asset&   value,
                 const name&    ram_payer );

      /**
       * Delete an account.
       *
       * @param acc - transfer from which account,
       * @param value - in order to get the symbol of currency.
       */
      [[eosio::action]]
      void close( const name&    acc,
                  const asset&   value );
      
      /**
       * Allows `from` account to transfer to `to` account the `quantity` tokens.
       * One account is debited and the other is credited with quantity tokens.
       *
       * @param from - transfer from which account,
       * @param to - transfer to which account,
       * @param quantity - the quantity of tokens to be transferred,
       * @param memo - the memo string to accompany the transaction.
       */
      [[eosio::action]]
      void transfer( const name&    from,
                     const name&    to,
                     const asset&   quantity,
                     const string&  memo );

      /**
       * Allows `from` account to transfer to `to` account the `quantity` tokens.
       * One account is debited and the other is credited with quantity tokens.
       *
       * @param from - transfer from which account,
       * @param to - transfer to which account,
       * @param quantity - the quantity of tokens to be transferred,
       * @param bcreate - create acc or not,
       * @param memo - the memo string to accompany the transaction.
       */
      [[eosio::action]]
      void yrctransfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        bool bcreate,
                        const string&  memo );

      /**
       * This action will approve of the loan.
       *
       * @param from - loan from which account,
       * @param manager - loan manager,
       * @param quantity - which asset.
       */
      [[eosio::action]]
      void approve( const name&   from,
                    const name&   manager,
                    const asset&  quantity );

      /**
       * This action will transfer the loan.
       *
       * @param manager - loan manager,
       * @param from - loan from which account,
       * @param to - transfer loan to which account,
       * @param quantity - which asset,
       * @param bcreate - create acc or not,
       * @param memo - the memo string.
       */
      [[eosio::action]]
      void loantrans( const name& manager,
                      const name& from,
                      const name& to,
                      const asset& quantity,
                      bool  bcreate,
                      const string& memo );

      /**
       * This action will add acc to tokenpool.
       *
       * @param user - which account,
       * @param value - which asset,
       * @param pool_name - the pool name,
       * @param memo - the memo string.
       */
      [[eosio::action]]
      void addtknpool( const name&   user,
                       const asset&  value,
                       const string& pool_name,
                       const string& memo );

      /**
       * This action will remove acc from tokenpool.
       *
       * @param user - which account,
       * @param value - which asset.
       */
      [[eosio::action]]
      void rmvtknpool( const name&  user,
                       const asset& value );

      /**
       * This action will add rule for lock.
       *
       * @param user - 具有资金池账号权限的用户,
       * @param lockruleid - id of the lock rule,
       * @param times - which account,
       * @param pcts - percentage's numerator,
       * @param base - percentage's denominator,
       * @param period - unlock period,
       * @param value - in order to get the ruler,
       * @param desc - the desc.
       */
      [[eosio::action]]
      void addrule(  const name& user,
                     uint32_t lockruleid,
                     const std::vector<uint64_t>& times,
                     const std::vector<uint16_t>& pcts,
                     uint32_t base,
                     uint32_t period,
                     const asset& value,
                     const string& desc );

      /**
       * This action will transfer a batch of asset.
       *
       * @param from - transfer from which account,
       * @param accs - transfer to which accounts,
       * @param amounts - transfer how many to every account,
       * @param value - in order to get the symbol,
       * @param memo - the memo.
       */
      [[eosio::action]]
      void batchtrans( const name&   from,
                       const std::vector<name>& accs,
                       const std::vector<int64_t>& amounts,
                       const asset& value,
                       const string& memo );

      /**
       * This action will transfer the locked asset.
       *
       * @param lockruleid - which lock rule,
       * @param from - transfer from which account,
       * @param to - transfer to which account,
       * @param quantity - quantity,
       * @param memo - the memo.
       */
      [[eosio::action]]
      void locktransfer( uint32_t      lockruleid,
                         const name&   from,
                         const name&   to,
                         const asset&  quantity,
                         const string& memo );

      /**
       * This action will unlock the asset of an account.
       *
       * @param acc - the account to be locked,
       * @param value - the asset of the account,
       * @param memo - the memo.
       */
      [[eosio::action]]
      void unlockasset( const name&    acc,
                      const asset&   value,
                      const string&  memo );

      static asset get_supply( const name& token_contract_account, const symbol_code& sym_code )
      {
         stats statstable( token_contract_account, sym_code.raw() );
         const auto& st = statstable.get( sym_code.raw() );
         return st.supply;
      }

      static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
      {
         accounts accountstable( token_contract_account, owner.value );
         const auto& ac = accountstable.get( sym_code.raw() );
         return ac.balance;
      }

      using setunicheck_action = eosio::action_wrapper<"setunicheck"_n, &yottatoken::setunicheck>;
      using create_action = eosio::action_wrapper<"create"_n, &yottatoken::create>;
      using issue_action = eosio::action_wrapper<"issue"_n, &yottatoken::issue>;
      using setextime_action = eosio::action_wrapper<"setextime"_n, &yottatoken::setextime>;
      using open_action = eosio::action_wrapper<"open"_n, &yottatoken::open>;
      using close_action = eosio::action_wrapper<"close"_n, &yottatoken::close>;
      using transfer_action = eosio::action_wrapper<"transfer"_n, &yottatoken::transfer>;
      using yrctransfer_action = eosio::action_wrapper<"yrctransfer"_n, &yottatoken::yrctransfer>;
      using approve_action = eosio::action_wrapper<"approve"_n, &yottatoken::approve>;
      using loantrans_action = eosio::action_wrapper<"loantrans"_n, &yottatoken::loantrans>;
      using addtknpool_action = eosio::action_wrapper<"addtknpool"_n, &yottatoken::addtknpool>;
      using rmvtknpool_action = eosio::action_wrapper<"rmvtknpool"_n, &yottatoken::rmvtknpool>;
      using addrule_action = eosio::action_wrapper<"addrule"_n, &yottatoken::addrule>;
      using batchtrans_action = eosio::action_wrapper<"batchtrans"_n, &yottatoken::batchtrans>;
      using locktransfer_action = eosio::action_wrapper<"locktransfer"_n, &yottatoken::locktransfer>;
      using unlockasset_action = eosio::action_wrapper<"unlockasset"_n, &yottatoken::unlockasset>;

   private:
      struct [[eosio::table]] reginfo {
         uint32_t    next_tokenno;
         uint32_t    tokens_count;
      };
      typedef eosio::singleton< "reginfo"_n, reginfo > reginfo_singleton;

      struct [[eosio::table]] unicheckname {
         name     contractacc;
      };
      typedef eosio::singleton< "unicheckname"_n, unicheckname > unicheck_singleton;

      struct [[eosio::table]] account {
         asset    balance;

         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "accounts"_n, account > accounts;

      struct [[eosio::table]] tokenpool {
         name     user;
         string   pool_name;
         string   memo;

         uint64_t primary_key()const { return user.value; }
      };
      typedef eosio::multi_index< "tokenpool"_n, tokenpool> tokenpools;

      struct [[eosio::table]] currency_stat {
         asset    supply;
         asset    max_supply;
         name     issuer;
         //name     ruler;
         name     poolsetter;
         name     unlocker;
         uint64_t time = 0; //exchanging time
         uint32_t tokenno; //token number

         uint64_t primary_key()const { return max_supply.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "stat"_n, currency_stat > stats;

      struct [[eosio::table]] assetkind {
         uint64_t symno; //symbol number in this contract
         uint32_t tokenno; //token number

         uint64_t primary_key()const { return symno; }
      };
      typedef eosio::multi_index< "assetkind"_n, assetkind > assetkinds;

      struct [[eosio::table]] lockrule {
         uint32_t                lockruleid;
         std::vector<uint64_t>   times;
         std::vector<uint16_t>   pcts; //lock percentage's numerator
         uint32_t                base; //lock percentage's denominator
         uint32_t                period;
         string                  desc;

         uint32_t                primary_key()const { return lockruleid; }
      };
      typedef eosio::multi_index< "lockrule"_n, lockrule> lockrules;

      struct [[eosio::table]] acclock {
         uint64_t        no_ruleid;
         asset           quantity;
         name            user;
         uint64_t        time;

         uint64_t primary_key()const { return no_ruleid; }
         uint64_t get_symbol() const { return quantity.symbol.code().raw(); };
      };
      typedef eosio::multi_index< "acclock"_n, acclock,
                                  eosio::indexed_by< "symbol"_n, eosio::const_mem_fun<acclock, uint64_t, &acclock::get_symbol> >
                                > acclocks;

      struct [[eosio::table]] numlock {
         name            user;
         asset           quantity;

         uint64_t        primary_key()const { return user.value; }
      };
      typedef eosio::multi_index< "numlock"_n, numlock> numlocks;

      struct [[eosio::table]] tokeninfo {
         uint32_t    tokenno;
         string      tokenname;
         string      issuechain; //issue in which chain
         name        contractacc; //account for the deployment contract
         string      contractname;
         string      contractadd;
         uint32_t    contractno;
         name        adminacc; //account of the administrator
         name        issuer;
         string      issuerurl;
         string      issuerid;
         string      issueradd;
         string      issuertel;
         uint16_t    precision;
         asset       supply;
         asset       max_supply;
         name        regacc; //account of the register
         uint64_t    regtime; //regist time
         uint64_t    updatetime;
         string      memo;

         uint64_t primary_key()const { return max_supply.symbol.code().raw(); }
         uint64_t get_issuer() const { return issuer.value; }
         uint64_t get_tokenno() const { return (uint64_t)tokenno; }
      };
      typedef eosio::multi_index< "tokeninfo"_n, tokeninfo,
                                  eosio::indexed_by< "issuer"_n, eosio::const_mem_fun<tokeninfo, uint64_t, &tokeninfo::get_issuer> >,
                                  eosio::indexed_by< "tokenno"_n, eosio::const_mem_fun<tokeninfo, uint64_t, &tokeninfo::get_tokenno> >
                                > tokeninfos;

      struct [[eosio::table]] authlevel {
         uint8_t         level;
         int64_t         amount;
         name            auth_name;

         uint8_t        primary_key()const { return level; }
      };
      typedef eosio::multi_index< "authlevel"_n, authlevel> authlevels;

      struct [[eosio::table]] loanpool {
         name            from;
         name            manager;
         asset           quantity;

         uint64_t        primary_key()const { return from.value; }
      };
      typedef eosio::multi_index< "loanpool"_n, loanpool> loanpools;

      struct [[eosio::table]] userreg {
         name        user;
         uint32_t    reg_count;
         uint32_t    total_count;
         uint32_t    next_tokenno;

         uint64_t primary_key()const { return user.value; }
      };
      typedef eosio::multi_index< "userreg"_n, userreg> userregs;

      void sub_balance( const name& owner, const asset& value );
      void add_balance( uint64_t namevalue, uint64_t symbol, const asset& value, const name& ram_payer, bool bcreate );
      asset get_lock_asset( const name& user, const asset& value );
};
