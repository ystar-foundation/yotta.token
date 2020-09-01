#include <yotta.token.hpp>

void yottatoken::setunicheck( const name& account )
{
   require_auth( get_self() );
   unicheck_singleton _unicheck( get_self(), get_self().value );
   check( !_unicheck.exists(), "uniqueness check contract account has benn setted" );
   unicheckname unicheckinfo = unicheckname{};
   unicheckinfo.contractacc = account;
   _unicheck.set( unicheckinfo, get_self() );
}

void yottatoken::create( const name& issuer, const asset&  maximum_supply, const string& token_name, const string& memo )
{
   require_auth( issuer );

   auto sym = maximum_supply.symbol;
   check( sym.is_valid(), "invalid symbol name" );
   check( maximum_supply.is_valid(), "invalid supply");
   check( maximum_supply.amount > 0, "max-supply must be positive");

   stats statstable( get_self(), sym.code().raw() );
   auto existing = statstable.find( sym.code().raw() );

   check( existing == statstable.end(), "token with symbol already exists" );
   check( token_name.size() <= 256, "token_name has more than 256 bytes" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );

   unicheck_singleton _unicheck( get_self(), get_self().value );
   check( _unicheck.exists(), "uniqueness check contract account has not benn setted" );
   auto regacc = _unicheck.get().contractacc;
   check( is_account( regacc ), "account does not exist");

   userregs _userreg( regacc, regacc.value );
   const auto& ur = _userreg.get( issuer.value, "You need to buy a token permission." );
   check( ur.total_count == ur.reg_count + 1, "You need to buy a new token permission again." );

   asset supply( 0, sym );
   auto tokenno = ur.next_tokenno;

   statstable.emplace(issuer, [&]( auto& s ) {
      s.tokenno       = tokenno;
      s.supply        = supply;
      s.max_supply    = maximum_supply;
      s.issuer        = issuer;
      s.poolsetter    = issuer;
      s.unlocker      = issuer;
   });

   auto acc_self = get_self();
   action( permission_level{acc_self, "active"_n}, "reg.token"_n, "simreg"_n, 
           std::make_tuple(acc_self, issuer, maximum_supply, token_name, memo) ).send();

}

void yottatoken::issue( const name& to, const asset& quantity, const string& memo )
{
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );
   check( is_account( to ), "to account does not exist");

   stats statstable( get_self(), sym.code().raw() );
   auto existing = statstable.find( sym.code().raw() );
   check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
   const auto& st = *existing;

   require_auth( st.issuer );
   check( quantity.is_valid(), "invalid quantity" );
   check( quantity.amount > 0, "must issue positive quantity" );
   check( quantity.symbol == st.supply.symbol, "symbol or precision mismatch" );
   check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

   statstable.modify( st, same_payer, [&]( auto& s ) {
      s.supply += quantity;
   });

   add_balance( to.value, sym.code().raw(), quantity, st.issuer, true );

   uint8_t mngtype = 2;
   auto acc_self = get_self();
   action( permission_level{acc_self, "active"_n}, "reg.token"_n, "updatesupply"_n, 
           std::make_tuple(mngtype, acc_self, acc_self, st.supply) ).send();
}

void yottatoken::setextime( uint64_t time, const asset& value )
{
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "This token is not existed when setextime." );

   check( st.time == 0, "The exchanging time has already been setted." );
   require_auth( st.issuer );

   statstable.modify( st, same_payer, [&]( auto& s ) {
      s.time = time;
   });
}

void yottatoken::open( const name& owner, const asset& value, const name& ram_payer )
{
   require_auth( ram_payer );
   check( is_account( owner ), "The account does not exist");
   accounts new_acnts( get_self(), owner.value );
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when newacc" );
   auto newacc = new_acnts.find( sym.code().raw() );
   check( newacc == new_acnts.end(), "This token has already been created." );
   asset accasset( 0, sym );
   new_acnts.emplace( ram_payer, [&]( auto& a ){
      a.balance = accasset;
   });
}

void yottatoken::close( const name& acc, const asset& value )
{
   require_auth( acc );
   check( is_account( acc ), "The account does not exist");
   accounts del_acnts( get_self(), acc.value );
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when delacc" );
   auto delacc = del_acnts.find( sym.code().raw() );
   check( delacc != del_acnts.end(), "This token doesn't exist." );
   check( delacc->balance.amount == 0, "The balance is not zero");
   del_acnts.erase( delacc );
}

void yottatoken::transfer( const name&    from,
                           const name&    to,
                           const asset&   quantity,
                           const string&  memo )
{
   check( from != to, "cannot transfer to self" );
   check( is_account( to ), "to account does not exist");
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol when transfer" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed." );

   require_auth( from );

   require_recipient( from );
   require_recipient( to );

   check( quantity.is_valid(), "invalid quantity" );
   check( quantity.amount > 0, "must transfer positive quantity" );
   check( quantity.symbol == st.supply.symbol, "symbol or precision mismatch" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );

   sub_balance( from, quantity );
   add_balance( to.value, sym.code().raw(), quantity, from, true );
}

void yottatoken::yrctransfer( const name&    from,
                           const name&    to,
                           const asset&   quantity,
                           bool bcreate,
                           const string&  memo )
{
   check( from != to, "cannot transfer to self" );
   check( is_account( to ), "to account does not exist");
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol when transfer" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed." );

   require_auth( from );

   require_recipient( from );
   require_recipient( to );

   check( quantity.is_valid(), "invalid quantity" );
   check( quantity.amount > 0, "must transfer positive quantity" );
   check( quantity.symbol == st.supply.symbol, "symbol or precision mismatch" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );

   sub_balance( from, quantity );
   add_balance( to.value, sym.code().raw(), quantity, from, bcreate );
}

void yottatoken::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );
   const auto& from_token = from_acnts.get( value.symbol.code().raw(), "Payer's token is not existed" );
   auto lock_asset = get_lock_asset(owner, value);
   check( lock_asset.symbol == value.symbol, "symbol or precision mismatch" );
   
   loanpools _loanpool( get_self(), value.symbol.code().raw() );
   auto loan = _loanpool.find( owner.value );
   if( loan == _loanpool.end() ) {
      check( from_token.balance.amount - lock_asset.amount >= value.amount, "overdrawn balance" );
   } else {
      check( from_token.balance.amount - lock_asset.amount - loan->quantity.amount  >= value.amount, "overdrawn balance" );
   }

   from_acnts.modify( from_token, owner, [&]( auto& a ) {
      a.balance -= value;
   });
}

void yottatoken::add_balance( uint64_t namevalue, uint64_t symbol, const asset& value, const name& ram_payer, bool bcreate )
{
   accounts to_acnts( get_self(), namevalue );
   auto to = to_acnts.find( symbol );
   if( to != to_acnts.end() ){
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
         a.balance.amount += value.amount;
      });
   } else if( bcreate ){
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      check( false, "Payee's token is not existed" );
   }
}

void yottatoken::approve( const name& from, const name& manager, const asset& quantity )
{
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol when approve" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed." );
   check( quantity.amount > 0, "must approve positive quantity" );
   check( quantity.symbol == st.supply.symbol, "symbol or precision mismatch" );

   require_auth( from );
   accounts from_acnts( get_self(), from.value );
   const auto& from_token = from_acnts.get( sym.code().raw(), "acc's token is not existed" );
   auto lock_asset = get_lock_asset(from, quantity);
   check( lock_asset.symbol == sym, "symbol or precision mismatch" );

   loanpools _loanpool( get_self(), sym.code().raw() );
   auto loan = _loanpool.find( from.value );
   if( loan == _loanpool.end() ) {
      check( from_token.balance.amount - lock_asset.amount >= quantity.amount, "overdrawn balance" );
      _loanpool.emplace(from, [&](auto &row) {
         row.from = from;
         row.manager = manager;
         row.quantity = quantity;
      });
   } else {
      check( loan->manager == manager, "manager should be the same as before");
      check( from_token.balance.amount - lock_asset.amount - loan->quantity.amount  >= quantity.amount, "overdrawn balance" );
      _loanpool.modify(loan, from, [&](auto &row) {
         row.quantity += quantity;
      });
   }
}

void yottatoken::loantrans( const name& manager, const name& from, const name& to, const asset& quantity, bool bcreate, const string& memo )
{
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol when loantrans" );
   check( quantity.amount > 0, "must loantrans positive quantity" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );
   require_auth( manager );
   loanpools _loanpool( get_self(), sym.code().raw() );
   const auto& loan = _loanpool.get( from.value, "loan is null" );
   check( loan.quantity.amount  >= quantity.amount, "overdrawn balance" );
   check( loan.manager == manager, "only manager can loantrans" );

   sub_balance( from, quantity );
   add_balance( to.value, sym.code().raw(), quantity, from, bcreate );

   if( loan.quantity.amount == quantity.amount ) {
      _loanpool.erase( loan );
   } else {
      _loanpool.modify(loan, same_payer, [&](auto &row) {
         row.quantity -= quantity;
      });
   }
}

void yottatoken::addtknpool( const name& user, const asset& value, const string& pool_name, const string& memo) {
   check( pool_name.size() <= 256, "pool_name has more than 256 bytes" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when addtknpool" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed when addtknpool" );
   require_auth( st.poolsetter );

   check( is_account( user ), "Account does not exist when addtknpool");
   tokenpools _tokenpool( get_self(), sym.code().raw() );
   auto poolacc = _tokenpool.find( user.value );
   check( poolacc == _tokenpool.end(), "user has already registered as token pool account" );  

   _tokenpool.emplace(st.poolsetter, [&](auto &row) {
      row.user = user;
      row.pool_name = pool_name;
      row.memo = memo;
   });
}

void yottatoken::rmvtknpool( const name& user, const asset& value ) {
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when rmvtknpool" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed when rmvtknpool" );
   require_auth( st.poolsetter );

   check( is_account( user ), "Account does not exist when rmvtknpool");
   tokenpools _tokenpool( get_self(), sym.code().raw() );
   const auto& poolacc = _tokenpool.get( user.value,  "is not a token pool account");
   _tokenpool.erase(poolacc);
}

void yottatoken::addrule( const name& user, uint32_t lockruleid, const std::vector<uint64_t>& times, const std::vector<uint16_t>& pcts,
                          uint32_t base, uint32_t period, const asset& value, const string& desc ) 
{
   require_auth( user );
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when addrule" );
   tokenpools _tokenpool( get_self(), sym.code().raw() );
   const auto& accpool = _tokenpool.get( user.value,  "is not a token pool account");

   check( lockruleid > 100, "lockruleid which less than 100 is reserved" );
   check( times.size() >= 1, "invalidate size of times array" ); //when equals one, lock by period
   check( times.size() == pcts.size(), "times and percentage in different size." );
   check( desc.size() <= 256, "desc has more than 256 bytes" );
   if ( times.size() == 1 ) {
      check( period > 0, "period must be a positive number" );
   }

   lockrules _lockrule( get_self(), sym.code().raw() );
   auto itrule = _lockrule.find(lockruleid);
   check( itrule == _lockrule.end(), "the id already existed in rule table" ); 

   for( size_t i = 0; i < times.size(); i++ ) {
      if( i == 0 ){
         check( pcts[i] >= 0 && pcts[i] <= base, "invalidate lock percentage" );
      } else {
         check( times[i] > times[i-1], "times vector error" );
         check( pcts[i] > pcts[i-1] && pcts[i] <= base, "lock percentage vector error" );
      }
   }

   _lockrule.emplace(user, [&](auto &row) {
      row.lockruleid   = lockruleid;
      row.times        = times;
      row.pcts         = pcts;
      row.base         = base;
      row.period       = period;
      row.desc         = desc;
   });
}

void yottatoken::batchtrans(const name& from, const std::vector<name>& accs,
                            const std::vector<int64_t>& amounts, const asset& value, const string& memo)
{
   require_auth( from );

   check( memo.size() <= 256, "memo has more than 256 bytes" );
   check( accs.size() == amounts.size(), "accounts and quantities in different size" );
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when batchtrans" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed when batchtrans." );
   check( sym == st.supply.symbol, "symbol or precision mismatch" );

   int64_t all_amount = 0;
   for(size_t no = 0; no < amounts.size(); no++) {
      accounts to_acnts( get_self(), accs[no].value );
      auto to = to_acnts.find( sym.code().raw() );
      if( to != to_acnts.end() && amounts[no] > 0 && is_account( accs[no]) ) {
         to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance.amount += amounts[no];
         });
         all_amount += amounts[no];
      }
   }
   asset subasset( all_amount, sym );
   sub_balance( from, subasset );
}

void yottatoken::locktransfer(uint32_t lockruleid, const name& from, const name& to, const asset& quantity, const string& memo) 
{
   require_auth( from );
   auto sym = quantity.symbol;
   check( sym.is_valid(), "invalid symbol when locktransfer" );
   check( quantity.amount > 0, "must locktransfer positive quantity" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );
   tokenpools _tokenpool( get_self(), sym.code().raw() );
   const auto& poolacc = _tokenpool.get( from.value, "only token pool account can locktransfer" );

   transfer( from, to, quantity, memo );

   if (lockruleid == 0) {
      numlocks _numlock( get_self(), sym.code().raw() );
      auto it = _numlock.find( to.value );
      if( it == _numlock.end() ) {
         _numlock.emplace(from, [&](auto &row) {
            row.user = to;
            row.quantity = quantity;
         });
      } else {
         _numlock.modify(it, from, [&](auto &row) {
            row.quantity += quantity;
         });
      }
      return;
   }
   lockrules _lockrule( get_self(), sym.code().raw() );
   const auto& itrule = _lockrule.get( lockruleid, "lockruleid not existed in rule table" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed" );
   uint64_t no_ruleid = lockruleid + ((uint64_t)st.tokenno << 32);
   acclocks _acclock( get_self(), to.value );
   auto _sym_lock = _acclock.get_index<"symbol"_n>();
   auto it = _sym_lock.find( sym.code().raw() );

   size_t rules_no = 0;
   while(it != _sym_lock.end() && it->quantity.symbol.code().raw() == sym.code().raw() ) {
      rules_no++;
      if (it->no_ruleid == no_ruleid) {
         _sym_lock.modify(it, same_payer, [&](auto &row) {
            row.time = current_time_point().sec_since_epoch();
            row.quantity.amount += quantity.amount;
         });
         return;
      }
      it++;
   }
   check( rules_no <= 100, "lock rules of account is too many" );
   auto itlc = _acclock.find(no_ruleid);
   if(itlc == _acclock.end()) {
      _acclock.emplace(from, [&](auto &row) {
         row.no_ruleid       = no_ruleid;
         row.quantity        = quantity;
         row.user            = to;
         row.time            = current_time_point().sec_since_epoch();
      });
   }
}

void yottatoken::unlockasset( const name& acc, const asset& value, const string& memo )
{
   auto sym = value.symbol;
   check( sym.is_valid(), "invalid symbol when unlockasset" );
   check( memo.size() <= 256, "memo has more than 256 bytes" );
   check( value.amount >= 0, "cannot lock negative quantity" );
   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed when unlockasset" );
   require_auth( st.unlocker );

   accounts _acnts( get_self(), acc.value );
   const auto& to = _acnts.get( sym.code().raw(),  "Account does not have this token");

   numlocks _numlock( get_self(), sym.code().raw() );
   const auto& it = _numlock.get( acc.value, "lockasset isn't existed" );
   check( it.quantity.amount >= value.amount, "locking asset should less than before" );
   if ( it.quantity.amount == value.amount ) {
      _numlock.erase( it );
   } else {
      _numlock.modify(it, st.unlocker, [&](auto &row) {
         row.quantity.amount -= value.amount;
      });
   }
}

asset yottatoken::get_lock_asset( const name& user, const asset& value )
{
   auto sym = value.symbol;
   asset lockasset( 0, sym );

   numlocks _numlock( get_self(), sym.code().raw() );
   auto il = _numlock.find( user.value );
   if( il != _numlock.end() )
      lockasset.amount = il->quantity.amount;

   stats statstable( get_self(), sym.code().raw() );
   const auto& st = statstable.get( sym.code().raw(), "token is not existed" );
   check( sym == st.supply.symbol, "symbol or precision mismatch" );
   acclocks _acclock( get_self(), user.value );
   uint64_t curtime = current_time_point().sec_since_epoch(); //seconds

   auto _sym_lock = _acclock.get_index<"symbol"_n>();
   auto it = _sym_lock.find( sym.code().raw() );

   while(it != _sym_lock.end() && it->quantity.symbol.code().raw() == sym.code().raw() ) {
      int64_t amount = it->quantity.amount;
      uint64_t extime = st.time; //exchanging time

      lockrules _lockrule( get_self(), sym.code().raw() );
      auto itrule = _lockrule.find(it->no_ruleid & 0xffffffff);
      if ( extime == 0 || itrule == _lockrule.end() || curtime <= extime) {
         lockasset.amount += amount;
      } else {
         uint32_t percent = 0;
         if ( itrule->times.size() == 1 ) { //lock by period
            int64_t numerator = (int64_t)curtime - (int64_t)extime - (int64_t)itrule->times[0];
            int64_t periods = numerator / (int64_t)itrule->period;
            if (numerator > 0 && periods >= 1) {
               percent = itrule->pcts[0] * periods;
               if (percent < itrule->base) {
                  percent = itrule->base - percent;
                  lockasset.amount += (int64_t)( (double)amount * percent / itrule->base);
               }
            } else {
               lockasset.amount += amount;
            }
         } else {
            size_t n = 0;
            for(auto itt = itrule->times.begin(); itt != itrule->times.end(); itt++) {
               if( extime + *itt > curtime ) {
                     break;
               }
               percent = itrule->pcts[n];
               n++;
            }
            percent = itrule->base - percent;
            lockasset.amount += (int64_t)( (double)amount * percent / itrule->base);
         }
      }

      it++;
   }

   return lockasset;
}
