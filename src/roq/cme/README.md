



2022-03-15 (testnet)
Unexpected updates with size 0
NEW
```
header={msg_type_raw="X", msg_type=MARKET_DATA_INCREMENTAL_REFRESH, sender_comp_id="DERIBITSERVER", target_comp_id="ROQ_TRADING", msg_seq_num=1454, sending_time=1647351337728ms},
market_data_incremental_refresh={symbol="BTC-PERPETUAL", md_req_id="roq-12", contract_multiplier=10, put_or_call=UNKNOWN, mark_price=38834.98, open_interest=299006933, no_md_entries=[
  {md_update_action=DELETE, md_entry_type=OFFER, md_entry_px=38836.5, md_entry_size=0, md_entry_date=1647351337727ms, deribit_trade_id="", side=UNKNOWN, order_id="", secondary_order_id="", ord_status=UNKNOWN, deribit_label="", index_price=nan, text="", deribit_liquidation=""},
  {md_update_action=NEW, md_entry_type=OFFER, md_entry_px=38842, md_entry_size=0, md_entry_date=1647351337727ms, deribit_trade_id="", side=UNKNOWN, order_id="", secondary_order_id="", ord_status=UNKNOWN, deribit_label="", index_price=nan, text="", deribit_liquidation=""}
], trade_volume24h=38833176}
```
CHANGE
```
header={msg_type_raw="X", msg_type=MARKET_DATA_INCREMENTAL_REFRESH, sender_comp_id="DERIBITSERVER", target_comp_id="ROQ_TRADING", msg_seq_num=856, sending_time=1647351208363ms},
market_data_incremental_refresh={symbol="BTC-PERPETUAL", md_req_id="roq-12", contract_multiplier=10, put_or_call=UNKNOWN, mark_price=38854.1, open_interest=299001667, no_md_entries=[
{md_update_action=CHANGE, md_entry_type=OFFER, md_entry_px=38865, md_entry_size=0, md_entry_date=1647351208362ms, deribit_trade_id="", side=UNKNOWN, order_id="", secondary_order_id="", ord_status=UNKNOWN, deribit_label="", index_price=nan, text="", deribit_liquidation=""}
], trade_volume24h=38883815}
```

















login="{
"method":"public/auth"
"params":{"grant_type":"client_signature"
"client_id":"5MP40u9h"
"timestamp":"1585292505911"
"nonce":"rvoh0cln98fay4v3z2kob1e50ca6tw7u"
"data":""
"signature":"69df60d2340c4492664f4787aa57b6469ee0e5b61757268593318b92d15579b2"}
"id":"login"}"

access_token="1616828505922.1HkGiwL8.-ekkSJW8OVCGnZgYR0Huz2txShzpz8Np6xZT4X4Wz9TnkAvg1R2_u9md2_IPDYkSATMlRT6Oo8ajDTTO0bHBfNQxJelidqIHjnyKUpBhCuBozx3qRBPfK8-6qCuax3Gi1mKycqPFUTEiLCt9komtKgK_uORr03XcmT1SYf916pDfeS6Slqj9ZD4frSaJBTuE6zxNXhkH-IrpOlx-soq5vN_Gd6MvI5Wiz7KoLR5hj2fEjDXrUBx1HHufWxNlnlDTkRkoEf_Zuy2-HV8gjuNLeHszET8"
expires_in=31536000
refresh_token="1616828505922.1E9uTu5U.oCu5jjTWObJ4U9Bk_aDS0cIaf-7bM9f8Av3SIXu6HSogu4BSI35EKVmJmlTu8_f2L33Lyz51yVyF-Db_QnuHUXZMsi4LEdZY7JdCS8NjbgVA287PyPGwfWwzu9LsQKCmvmQWad5TtrnMVScC1ZmnUtL8Zt-p-UcbXhOGOljHI268Z2aGVLGy1GvggAeudEP93nXadHu5VTEguHX8_7OiP-x2aTQhQNCJC8260zSTgfZM7D12Sx9X1se4vrm8P_6zLNA213mHhYSElt2P522a1bBPbg"
scope="account:read block_trade:read connection mainaccount trade:read_write wallet:read"
state=""
token_type="bearer"


login="{
"method":"public/auth"
"params":{"grant_type":"client_signature"
"client_id":"5MP40u9h"
"timestamp":"1585292596820"
"nonce":"ijdm0e9osr9zm9ff2m2ce3v764seu"
"data":""
"signature":"8f95b8e6816fce9e318265f245e49ddc0b70e20428c3bedf916e1d9917e86217"}
"id":"login"}"

C0327 07:03:16.848638 27530 web_socket.cpp:193] error={code=-32700, message="Parse error", description=""}, id=""
