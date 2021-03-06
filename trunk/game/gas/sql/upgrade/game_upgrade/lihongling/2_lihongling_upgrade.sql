drop table if exists tbl_user_charge_detail;
drop table if exists tbl_toolsmall_item_geter;
drop table if exists tbl_toolsmall_item_buyed;
drop table if exists tbl_toolsmall_buy_detail;
create table tbl_toolsmall_item_buyed
(	
	tib_uDetailID		bigint unsigned	 not	null auto_increment,	#detail_id
	cs_uId					bigint unsigned not null, #buyer id
	is_uType				tinyint unsigned not	null,		#item type
	is_sName				varchar(96)			not	null,		#item name
	tib_uCount			bigint unsigned not null, #buy count
	tib_dtTime			datetime not null,
	primary key(tib_uDetailID),
	key(tib_dtTime),
	foreign	key(cs_uId)	references tbl_char_static(cs_uId) on update cascade on delete cascade
)engine=innodb;

#��������
create table tbl_toolsmall_item_geter
(
	tib_uDetailID		bigint unsigned not null,	#order id
	cs_uId					bigint unsigned not null,	#buyer id
	unique key(tib_uDetailID),
	foreign	key(cs_uId)	references tbl_char_static(cs_uId) on update cascade on delete cascade,
	foreign	key(tib_uDetailID)	references tbl_toolsmall_item_buyed(tib_uDetailID) on	update cascade on delete cascade
)engine=innodb;