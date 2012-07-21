/*
 * finance.cc
 *
 * Copyright (c) 1997 - 2012 Hansjörg Malthaner and
 *        the simutrans development team
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#include <stdio.h>
#include <assert.h> 

#include "../simworld.h"
#include "../besch/haus_besch.h"
#include "../dataobj/loadsave.h"
#include "../dataobj/scenario.h"
#include "simplay.h"

#include "finance.h"


finance_t::finance_t(spieler_t * _player, karte_t * _world) :
	player(_player),
	world(_world)
{
	konto = world->get_settings().get_starting_money(world->get_last_year());
	starting_money = konto;
	konto_ueberzogen = 0;

	/**
	 * initialize finance history arrays
	 * @author Jan Korbel
	 */
	for (int year=0; year<MAX_PLAYER_HISTORY_YEARS2; year++) {
		for (int cost_type=0; cost_type<ATC_MAX; cost_type++) {
			com_year[year][cost_type] = 0;
			if ((cost_type == ATC_CASH) || (cost_type == ATC_NETWEALTH)) {
				com_year[year][cost_type] = starting_money;
			}
		}
	}

	for (int month=0; month<MAX_PLAYER_HISTORY_MONTHS2; month++) {
		for (int cost_type=0; cost_type<ATC_MAX; cost_type++) {
			com_month[month][cost_type] = 0;
			if ((cost_type == ATC_CASH) || (cost_type == ATC_NETWEALTH)) {
				com_month[month][cost_type] = starting_money;
			}
		}
	}

	for (int transport_type=0; transport_type<TT_MAX; ++transport_type){
		for (int year=0; year<MAX_PLAYER_HISTORY_YEARS2; year++) {
			for (int cost_type=0; cost_type<ATV_MAX; cost_type++) {
				veh_year[transport_type][year][cost_type] = 0;
			}
		}
	}

	for (int transport_type=0; transport_type<TT_MAX; ++transport_type){
		for (int month=0; month<MAX_PLAYER_HISTORY_MONTHS2; month++) {
			for (int cost_type=0; cost_type<ATV_MAX; cost_type++) {
				veh_month[transport_type][month][cost_type] = 0;
			}
		}
	}

	for(int i=0; i<TT_MAX; ++i){
		maintenance[i] = 0;
	}

	for(int i=0; i<TT_MAX_VEH; ++i){
		vehicle_maintenance[i] = 0;
	}

}

/*
void finance_t::book_convoi_number( int count ) {
	finance.com_year[0][ATC_ALL_CONVOIS] += count;
	finance.com_month[0][ATC_ALL_CONVOIS] += count;
}
*/

void finance_t::calc_finance_history() {
	// vehicles
	for(int tt=1; tt<TT_MAX; ++tt){
		// ATV_REVENUE_TRANSPORT = ATV_REVENUE_PAS+MAIL+GOOD
		sint64 revenue, mrevenue;
		revenue = mrevenue = 0;
		for(int i=0; i<ATV_REVENUE_TRANSPORT; ++i){
			mrevenue += veh_month[tt][0][i];
			revenue  += veh_year[ tt][0][i];
		}
		veh_month[tt][0][ATV_REVENUE_TRANSPORT] = mrevenue;
		veh_year[ tt][0][ATV_REVENUE_TRANSPORT] = revenue;

		// ATV_REVENUE = ATV_REVENUE_TRANSPORT + ATV_TOLL_RECEIVED
		veh_month[tt][0][ATV_REVENUE] = veh_month[tt][0][ATV_REVENUE_TRANSPORT] + veh_month[tt][0][ATV_TOLL_RECEIVED];
		veh_year[tt][0][ATV_REVENUE] = veh_year[tt][0][ATV_REVENUE_TRANSPORT] + veh_year[tt][0][ATV_TOLL_RECEIVED];

		// ATC_EXPENDITURE = ATC_RUNNIG_COST + ATC_VEH_MAINTENENCE + ATC_INF_MAINTENENCE + ATC_TOLL_PAYED;
		sint64 expenditure, mexpenditure;
		expenditure = mexpenditure = 0;
		for(int i=ATV_RUNNING_COST; i<ATV_EXPENDITURE; ++i){
			mexpenditure += veh_month[tt][0][i];
			expenditure  += veh_year[ tt][0][i];
		}
		veh_month[tt][0][ATV_EXPENDITURE] = mexpenditure;
		veh_year[ tt][0][ATV_EXPENDITURE] = expenditure;
		veh_month[tt][0][ATV_OPERATING_PROFIT] = veh_month[tt][0][ATV_REVENUE] + mexpenditure;
		veh_year[ tt][0][ATV_OPERATING_PROFIT] = veh_year[ tt][0][ATV_REVENUE] +  expenditure;

		// PROFIT = OPERATING_PROFIT + NEW_VEHICLES + construction costs 
		sint64 profit, mprofit;
		profit = mprofit = 0;
		for(int i=ATV_OPERATING_PROFIT; i<ATV_PROFIT; ++i){
			mprofit += veh_month[tt][0][i];
			profit  += veh_year[ tt][0][i];
		}
		veh_month[tt][0][ATV_PROFIT] = mprofit;
		veh_year[ tt][0][ATV_PROFIT] =  profit;

		veh_month[tt][0][ATV_WAY_TOLL] = veh_month[tt][0][ATV_TOLL_RECEIVED] + veh_month[tt][0][ATV_TOLL_PAYED]; 
		veh_year[ tt][0][ATV_WAY_TOLL] = veh_year[tt][0][ATV_TOLL_RECEIVED] + veh_year[tt][0][ATV_TOLL_PAYED]; 

		veh_month[tt][0][ATV_PROFIT_MARGIN] = calc_margin(veh_month[tt][0][ATV_OPERATING_PROFIT], veh_month[tt][0][ATV_REVENUE]);
		veh_year[tt][0][ATV_PROFIT_MARGIN] = calc_margin(veh_year[tt][0][ATV_OPERATING_PROFIT], veh_year[tt][0][ATV_REVENUE]);

		sint64 transported, mtransported;
		transported = mtransported = 0;
		for(int i=ATV_TRANSPORTED_PASSENGER; i<ATV_TRANSPORTED; ++i){
			mtransported += veh_month[tt][0][i];
			transported  += veh_year[ tt][0][i];
		}
		veh_month[tt][0][ATV_TRANSPORTED] = mtransported;
		veh_year[ tt][0][ATV_TRANSPORTED] =  transported;
	}

	// sum up statistic for all transport types
	for( int j=0; j< ATV_MAX; ++j ) {
		veh_month[TT_ALL][0][j] =0;
		for( int tt=1; tt<TT_MAX; ++tt ) {
			// do not add poverline revenue to vehicles revenue
			if ( ( tt != TT_POWERLINE ) || ( j >= ATV_REVENUE )) {
				veh_month[TT_ALL][0][j] += veh_month[tt][0][j];
			}
		}
	}
	for( int j=0; j< ATV_MAX; ++j ) {
		veh_year[TT_ALL][0][j] =0;
		for( int tt=1; tt<TT_MAX; ++tt ) {
			// do not add poverline revenue to vehicles revenue
			if ( ( tt != TT_POWERLINE ) || ( j >= ATV_REVENUE )) {
				veh_year[TT_ALL][0][j] += veh_year[tt][0][j];
			}
		}
	}
	// recalc margin for TT_ALL
	veh_month[TT_ALL][0][ATV_PROFIT_MARGIN] = calc_margin(veh_month[TT_ALL][0][ATV_OPERATING_PROFIT], veh_month[TT_ALL][0][ATV_REVENUE]);
	veh_year[TT_ALL][0][ATV_PROFIT_MARGIN] = calc_margin(veh_year[TT_ALL][0][ATV_OPERATING_PROFIT], veh_year[TT_ALL][0][ATV_REVENUE]);

	// undistinguishable by type of transport 
	com_month[0][ATC_CASH] = konto;
	com_year [0][ATC_CASH] = konto;
	com_month[0][ATC_NETWEALTH] = veh_month[TT_ALL][0][ATV_NON_FINANTIAL_ASSETS] + konto;
	com_year [0][ATC_NETWEALTH] = veh_year[TT_ALL][0][ATV_NON_FINANTIAL_ASSETS] + konto;
	com_month[0][ATC_SCENARIO_COMPLETED] = com_year[0][ATC_SCENARIO_COMPLETED] = world->get_scenario()->completed(player->get_player_nr());

}


void finance_t::calc_flat_view_month(int tt, sint64 (&flat_view_month)[MAX_PLAYER_HISTORY_MONTHS][MAX_PLAYER_COST]){
	assert((0 <=tt ) && ( tt < TT_MAX ));
	for(int month=0; month<MAX_PLAYER_HISTORY_MONTHS; ++month) {
		for(int i=0; i<MAX_PLAYER_COST; ++i) {
			int index = translate_index_cost_to_at(i);
			if(index >=0 ){
				flat_view_month[month][i] = veh_month[tt][month][index];
			} else {
				if(i==COST_CASH) {
					flat_view_month[month][i] = com_month[month][ATC_CASH];
				}
				if(i==COST_NETWEALTH ) {
					flat_view_month[month][i] = com_month[month][ATC_NETWEALTH];
				}
				if( ( i == COST_POWERLINES ) && ( tt == TT_ALL ) ) {
					flat_view_month[month][i] = veh_month[TT_POWERLINE][month][ATV_REVENUE];
				}
			}
		}
	}
}

void finance_t::calc_flat_view_year( int tt, sint64 (&flat_view_year)[ MAX_PLAYER_HISTORY_YEARS ][MAX_PLAYER_COST]){
	assert(( 0<=tt ) && ( tt < TT_MAX ));
	for(int year=0; year<MAX_PLAYER_HISTORY_YEARS; ++year) {
		for(int i=0; i<MAX_PLAYER_COST; ++i) {
			int index = translate_index_cost_to_at(i);
			if(index >=0 ){
				flat_view_year[year][i] = veh_year[tt][year][index];
			} else {
				if(i==COST_CASH) {
					flat_view_year[year][i] = com_year[year][ATC_CASH];
				}
				if(i==COST_NETWEALTH ) {
					flat_view_year[year][i] = com_year[year][ATC_NETWEALTH];
				}
				if( ( i == COST_POWERLINES ) && ( tt == TT_ALL ) ) {
					flat_view_year[year][i] = veh_year[TT_POWERLINE][year][ATV_REVENUE];
				}
			}
		}
	}
}


void finance_t::export_to_cost_month(sint64 (&finance_history_month)[MAX_PLAYER_HISTORY_MONTHS][MAX_PLAYER_COST]) {
	calc_finance_history();
	for(int i=0; i<MAX_PLAYER_HISTORY_MONTHS; ++i){
		finance_history_month[i][COST_CONSTRUCTION] = veh_month[TT_ALL][i][ATV_CONSTRUCTION_COST];
		finance_history_month[i][COST_VEHICLE_RUN]  = veh_month[TT_ALL][i][ATV_RUNNING_COST] + veh_month[TT_ALL][i][ATV_VEHICLE_MAINTENANCE];
		finance_history_month[i][COST_NEW_VEHICLE]  = veh_month[TT_ALL][i][ATV_NEW_VEHICLE];
		finance_history_month[i][COST_INCOME]       = veh_month[TT_ALL][i][ATV_REVENUE_TRANSPORT];
		finance_history_month[i][COST_MAINTENANCE]  = veh_month[TT_ALL][i][ATV_INFRASTRUCTURE_MAINTENANCE];
		finance_history_month[i][COST_ASSETS]       = veh_month[TT_ALL][i][ATV_NON_FINANTIAL_ASSETS];
		finance_history_month[i][COST_CASH]         = com_month[i][ATC_CASH];
		finance_history_month[i][COST_NETWEALTH]    = com_month[i][ATC_NETWEALTH];
		finance_history_month[i][COST_PROFIT]       = veh_month[TT_ALL][i][ATV_PROFIT];
		finance_history_month[i][COST_OPERATING_PROFIT] = veh_month[TT_ALL][i][ATV_OPERATING_PROFIT];
		finance_history_month[i][COST_MARGIN]           = veh_month[TT_ALL][i][ATV_PROFIT_MARGIN];
		finance_history_month[i][COST_ALL_TRANSPORTED]  = veh_month[TT_ALL][i][ATV_TRANSPORTED];
		finance_history_month[i][COST_POWERLINES]       = veh_month[TT_POWERLINE][i][ATV_REVENUE];
		finance_history_month[i][COST_TRANSPORTED_PAS]  = veh_month[TT_ALL][i][ATV_TRANSPORTED_PASSENGER];
		finance_history_month[i][COST_TRANSPORTED_MAIL] = veh_month[TT_ALL][i][ATV_TRANSPORTED_MAIL];
		finance_history_month[i][COST_TRANSPORTED_GOOD] = veh_month[TT_ALL][i][ATV_TRANSPORTED_GOOD];
		finance_history_month[i][COST_ALL_CONVOIS]      = com_month[i][ATC_ALL_CONVOIS];
		finance_history_month[i][COST_SCENARIO_COMPLETED] = com_month[i][ATC_SCENARIO_COMPLETED];
		finance_history_month[i][COST_WAY_TOLLS]        = veh_month[TT_ALL][i][ATV_WAY_TOLL];
	}
}


void finance_t::export_to_cost_year( sint64 (&finance_history_year)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]) {
	calc_finance_history();
	for(int i=0; i<MAX_PLAYER_HISTORY_YEARS; ++i){
		finance_history_year[i][COST_CONSTRUCTION] = veh_year[TT_ALL][i][ATV_CONSTRUCTION_COST];
		finance_history_year[i][COST_VEHICLE_RUN]  = veh_year[TT_ALL][i][ATV_RUNNING_COST] + veh_month[TT_ALL][i][ATV_VEHICLE_MAINTENANCE];
		finance_history_year[i][COST_NEW_VEHICLE]  = veh_year[TT_ALL][i][ATV_NEW_VEHICLE];
		finance_history_year[i][COST_INCOME]       = veh_year[TT_ALL][i][ATV_REVENUE_TRANSPORT];
		finance_history_year[i][COST_MAINTENANCE]  = veh_year[TT_ALL][i][ATV_INFRASTRUCTURE_MAINTENANCE];
		finance_history_year[i][COST_ASSETS]       = veh_year[TT_ALL][i][ATV_NON_FINANTIAL_ASSETS];
		finance_history_year[i][COST_CASH]         = com_year[i][ATC_CASH];
		finance_history_year[i][COST_NETWEALTH]    = com_year[i][ATC_NETWEALTH];
		finance_history_year[i][COST_PROFIT]       = veh_year[TT_ALL][i][ATV_PROFIT];
		finance_history_year[i][COST_OPERATING_PROFIT] = veh_year[TT_ALL][i][ATV_OPERATING_PROFIT];
		finance_history_year[i][COST_MARGIN]           = veh_year[TT_ALL][i][ATV_PROFIT_MARGIN];
		finance_history_year[i][COST_ALL_TRANSPORTED]  = veh_year[TT_ALL][i][ATV_TRANSPORTED];
		finance_history_year[i][COST_POWERLINES]       = veh_year[TT_POWERLINE][i][ATV_REVENUE];
		finance_history_year[i][COST_TRANSPORTED_PAS]  = veh_year[TT_ALL][i][ATV_TRANSPORTED_PASSENGER];
		finance_history_year[i][COST_TRANSPORTED_MAIL] = veh_year[TT_ALL][i][ATV_TRANSPORTED_MAIL];
		finance_history_year[i][COST_TRANSPORTED_GOOD] = veh_year[TT_ALL][i][ATV_TRANSPORTED_GOOD];
		finance_history_year[i][COST_ALL_CONVOIS]      = com_year[i][ATC_ALL_CONVOIS];
		finance_history_year[i][COST_SCENARIO_COMPLETED] = com_year[i][ATC_SCENARIO_COMPLETED];
		finance_history_year[i][COST_WAY_TOLLS]        = veh_year[TT_ALL][i][ATV_WAY_TOLL];
	}
}


/* 
 * int tt is COST_ !!! 
*/
sint64 finance_t::get_finance_history_year(int tt, int year, int type) { 
	assert((tt>=0) && (tt<TT_MAX));
	int index = translate_index_cost_to_at(type);
	const int atc_index = translate_index_cost_to_atc(type);
	assert(index < ATV_MAX);
	assert(atc_index < ATC_MAX);

	if( ( tt == TT_ALL ) && ( type == COST_POWERLINES ) ) {
		return veh_year[TT_POWERLINE][year][ATV_REVENUE];
	}
	if( index >= 0 ) {
		return veh_year[tt][year][index];
	}
	else { 
		return ( atc_index >= 0 ) ? com_year[year][atc_index] : 0; 
	}
}


/* 
 * int tt is COST_ !!! 
*/
sint64 finance_t::get_finance_history_month(int tt, int month, int type) { 
	assert((tt>=0) && (tt<TT_MAX));
	int index = translate_index_cost_to_at(type);
	const int atc_index = translate_index_cost_to_atc(type);
	assert( index < ATV_MAX );
	assert( atc_index < ATC_MAX );

	if( ( tt == TT_ALL ) && ( type == COST_POWERLINES ) ) {
		return veh_month[TT_POWERLINE][month][ATV_REVENUE];
	}
	if( index >= 0 ) {
		return veh_month[tt][month][index];
	}
	else { 
		return ( atc_index >= 0 ) ? com_month[month][atc_index] : 0; 
	}
}


sint64 finance_t::get_maintenance_with_bits(transport_type tt) const { 
	assert(tt<TT_MAX); 

	if(  world->ticks_per_world_month_shift>=18  ) {
		return ((sint64)maintenance[tt]) << (world->ticks_per_world_month_shift-18);
	}else{
		return ((sint64)maintenance[tt]) >> (18-world->ticks_per_world_month_shift);
	}
}


sint64 finance_t::get_vehicle_maintenance_with_bits(transport_type tt) const { 
	assert(tt<TT_MAX); 

	if(  world->ticks_per_world_month_shift>=18  ) {
		return ((sint64)vehicle_maintenance[tt]) << (world->ticks_per_world_month_shift-18);
	}else{
		return ((sint64)vehicle_maintenance[tt]) >> (18-world->ticks_per_world_month_shift);
	}
}



void finance_t::import_from_cost_month(const sint64 (& finance_history_month)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]) {
	// does it need initial clean-up ? (= initialization)
	for(int i=0; i<MAX_PLAYER_HISTORY_MONTHS; ++i){
		veh_month[TT_OTHER][i][ATV_CONSTRUCTION_COST] = finance_history_month[i][COST_CONSTRUCTION];
		veh_month[TT_ALL  ][i][ATV_CONSTRUCTION_COST] = finance_history_month[i][COST_CONSTRUCTION];
		veh_month[TT_OTHER][i][ATV_RUNNING_COST]      = finance_history_month[i][COST_VEHICLE_RUN];
		veh_month[TT_ALL  ][i][ATV_RUNNING_COST]      = finance_history_month[i][COST_VEHICLE_RUN];
		veh_month[TT_OTHER][i][ATV_NEW_VEHICLE]       = finance_history_month[i][COST_NEW_VEHICLE];
		veh_month[TT_ALL  ][i][ATV_NEW_VEHICLE]       = finance_history_month[i][COST_NEW_VEHICLE];
		// he have to store it in _GOOD for not being override in calc_finance history() to 0
		veh_month[TT_OTHER][i][ATV_REVENUE_GOOD]      = finance_history_month[i][COST_INCOME];
		veh_month[TT_ALL  ][i][ATV_REVENUE_GOOD]      = finance_history_month[i][COST_INCOME];
		veh_month[TT_OTHER][i][ATV_REVENUE_TRANSPORT]      = finance_history_month[i][COST_INCOME];
		veh_month[TT_ALL  ][i][ATV_REVENUE_TRANSPORT]      = finance_history_month[i][COST_INCOME];
		veh_month[TT_OTHER][i][ATV_INFRASTRUCTURE_MAINTENANCE] = finance_history_month[i][COST_MAINTENANCE];
		veh_month[TT_ALL  ][i][ATV_INFRASTRUCTURE_MAINTENANCE] = finance_history_month[i][COST_MAINTENANCE];
		veh_month[TT_OTHER][i][ATV_NON_FINANTIAL_ASSETS] = finance_history_month[i][COST_ASSETS];
		veh_month[TT_ALL  ][i][ATV_NON_FINANTIAL_ASSETS] = finance_history_month[i][COST_ASSETS];
		com_month[i][ATC_CASH]                        = finance_history_month[i][COST_CASH];
		com_month[i][ATC_NETWEALTH]                   = finance_history_month[i][COST_NETWEALTH];
		veh_month[TT_OTHER][i][ATV_PROFIT]            = finance_history_month[i][COST_PROFIT];
		veh_month[TT_ALL  ][i][ATV_PROFIT]            = finance_history_month[i][COST_PROFIT];
		veh_month[TT_OTHER][i][ATV_OPERATING_PROFIT]  = finance_history_month[i][COST_OPERATING_PROFIT];
		veh_month[TT_ALL  ][i][ATV_OPERATING_PROFIT]  = finance_history_month[i][COST_OPERATING_PROFIT];
		veh_month[TT_ALL  ][i][ATV_PROFIT_MARGIN]     = finance_history_month[i][COST_MARGIN]; // this needs to be recalculate before usage
		veh_month[TT_OTHER][i][ATV_TRANSPORTED]       = finance_history_month[i][COST_ALL_TRANSPORTED];
		veh_month[TT_ALL  ][i][ATV_TRANSPORTED]       = finance_history_month[i][COST_ALL_TRANSPORTED];
		veh_month[TT_POWERLINE][i][ATV_REVENUE]       = finance_history_month[i][COST_POWERLINES];
		veh_month[TT_OTHER][i][ATV_TRANSPORTED_PASSENGER] = finance_history_month[i][COST_TRANSPORTED_PAS];
		veh_month[TT_ALL  ][i][ATV_TRANSPORTED_PASSENGER] = finance_history_month[i][COST_TRANSPORTED_PAS];
		veh_month[TT_OTHER][i][ATV_TRANSPORTED_MAIL]  = finance_history_month[i][COST_TRANSPORTED_MAIL];
		veh_month[TT_ALL  ][i][ATV_TRANSPORTED_MAIL]  = finance_history_month[i][COST_TRANSPORTED_MAIL];
		veh_month[TT_OTHER][i][ATV_TRANSPORTED_GOOD]  = finance_history_month[i][COST_TRANSPORTED_GOOD];
		veh_month[TT_ALL  ][i][ATV_TRANSPORTED_GOOD]  = finance_history_month[i][COST_TRANSPORTED_GOOD];
		com_month[i][ATC_ALL_CONVOIS]                 = finance_history_month[i][COST_ALL_CONVOIS];
		com_month[i][ATC_SCENARIO_COMPLETED]          = finance_history_month[i][COST_SCENARIO_COMPLETED];
		if(finance_history_month[i][COST_WAY_TOLLS] > 0 ){
			veh_month[TT_OTHER][i][ATV_TOLL_RECEIVED] = finance_history_month[i][COST_WAY_TOLLS];
			veh_month[TT_ALL  ][i][ATV_TOLL_RECEIVED] = finance_history_month[i][COST_WAY_TOLLS];
		}else{
			veh_month[TT_OTHER][i][ATV_TOLL_PAYED] = finance_history_month[i][COST_WAY_TOLLS];
			veh_month[TT_ALL  ][i][ATV_TOLL_PAYED] = finance_history_month[i][COST_WAY_TOLLS];
		}
		veh_month[TT_OTHER][i][ATV_WAY_TOLL] = finance_history_month[i][COST_WAY_TOLLS];
		veh_month[TT_ALL  ][i][ATV_WAY_TOLL] = finance_history_month[i][COST_WAY_TOLLS];
	}
}


void finance_t::import_from_cost_year( const sint64 (& finance_history_year)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]) {
	for(int i=0; i<MAX_PLAYER_HISTORY_YEARS; ++i){
		veh_year[TT_OTHER][i][ATV_CONSTRUCTION_COST] = finance_history_year[i][COST_CONSTRUCTION];
		veh_year[TT_ALL  ][i][ATV_CONSTRUCTION_COST] = finance_history_year[i][COST_CONSTRUCTION];
		veh_year[TT_OTHER][i][ATV_RUNNING_COST]      = finance_history_year[i][COST_VEHICLE_RUN];
		veh_year[TT_ALL  ][i][ATV_RUNNING_COST]      = finance_history_year[i][COST_VEHICLE_RUN];
		veh_year[TT_OTHER][i][ATV_NEW_VEHICLE]       = finance_history_year[i][COST_NEW_VEHICLE];
		veh_year[TT_ALL  ][i][ATV_NEW_VEHICLE]       = finance_history_year[i][COST_NEW_VEHICLE];
		// he have to store it in _GOOD for not being override in calc_finance history() to 0
		veh_year[TT_OTHER][i][ATV_REVENUE_GOOD]      = finance_history_year[i][COST_INCOME];
		veh_year[TT_ALL  ][i][ATV_REVENUE_GOOD]      = finance_history_year[i][COST_INCOME];
		veh_year[TT_OTHER][i][ATV_REVENUE_TRANSPORT]      = finance_history_year[i][COST_INCOME];
		veh_year[TT_ALL  ][i][ATV_REVENUE_TRANSPORT]      = finance_history_year[i][COST_INCOME];
		veh_year[TT_OTHER][i][ATV_INFRASTRUCTURE_MAINTENANCE] = finance_history_year[i][COST_MAINTENANCE];
		veh_year[TT_ALL  ][i][ATV_INFRASTRUCTURE_MAINTENANCE] = finance_history_year[i][COST_MAINTENANCE];
		veh_year[TT_OTHER][i][ATV_NON_FINANTIAL_ASSETS] = finance_history_year[i][COST_ASSETS];
		veh_year[TT_ALL  ][i][ATV_NON_FINANTIAL_ASSETS] = finance_history_year[i][COST_ASSETS];
		com_year[i][ATC_CASH]                        = finance_history_year[i][COST_CASH];
		com_year[i][ATC_NETWEALTH]                   = finance_history_year[i][COST_NETWEALTH];
		veh_year[TT_OTHER][i][ATV_PROFIT]            = finance_history_year[i][COST_PROFIT];
		veh_year[TT_ALL  ][i][ATV_PROFIT]            = finance_history_year[i][COST_PROFIT];
		veh_year[TT_OTHER][i][ATV_OPERATING_PROFIT]  = finance_history_year[i][COST_OPERATING_PROFIT];
		veh_year[TT_ALL  ][i][ATV_OPERATING_PROFIT]  = finance_history_year[i][COST_OPERATING_PROFIT];
		veh_year[TT_ALL  ][i][ATV_PROFIT_MARGIN]     = finance_history_year[i][COST_MARGIN]; // this needs to be recalculate before usage
		veh_year[TT_OTHER][i][ATV_TRANSPORTED]       = finance_history_year[i][COST_ALL_TRANSPORTED];
		veh_year[TT_ALL  ][i][ATV_TRANSPORTED]       = finance_history_year[i][COST_ALL_TRANSPORTED];
		veh_year[TT_POWERLINE][i][ATV_REVENUE]       = finance_history_year[i][COST_POWERLINES];
		veh_year[TT_OTHER][i][ATV_TRANSPORTED_PASSENGER] = finance_history_year[i][COST_TRANSPORTED_PAS];
		veh_year[TT_ALL  ][i][ATV_TRANSPORTED_PASSENGER] = finance_history_year[i][COST_TRANSPORTED_PAS];
		veh_year[TT_OTHER][i][ATV_TRANSPORTED_MAIL]  = finance_history_year[i][COST_TRANSPORTED_MAIL];
		veh_year[TT_ALL  ][i][ATV_TRANSPORTED_MAIL]  = finance_history_year[i][COST_TRANSPORTED_MAIL];
		veh_year[TT_OTHER][i][ATV_TRANSPORTED_GOOD]  = finance_history_year[i][COST_TRANSPORTED_GOOD];
		veh_year[TT_ALL  ][i][ATV_TRANSPORTED_GOOD]  = finance_history_year[i][COST_TRANSPORTED_GOOD];
		com_year[i][ATC_ALL_CONVOIS]                 = finance_history_year[i][COST_ALL_CONVOIS];
		com_year[i][ATC_SCENARIO_COMPLETED]          = finance_history_year[i][COST_SCENARIO_COMPLETED];
		if(finance_history_year[i][COST_WAY_TOLLS] > 0 ){
			veh_year[TT_OTHER][i][ATV_TOLL_RECEIVED] = finance_history_year[i][COST_WAY_TOLLS];
			veh_year[TT_ALL  ][i][ATV_TOLL_RECEIVED] = finance_history_year[i][COST_WAY_TOLLS];
		}else{
			veh_year[TT_OTHER][i][ATV_TOLL_PAYED] = finance_history_year[i][COST_WAY_TOLLS];
			veh_year[TT_ALL  ][i][ATV_TOLL_PAYED] = finance_history_year[i][COST_WAY_TOLLS];
		}
		veh_year[TT_OTHER][i][ATV_WAY_TOLL] = finance_history_year[i][COST_WAY_TOLLS];
		veh_year[TT_ALL  ][i][ATV_WAY_TOLL] = finance_history_year[i][COST_WAY_TOLLS];
	}
}


bool finance_t::is_bancrupted() const {
	return (
		com_year[0][ATC_NETWEALTH] <=0  &&
		veh_year[TT_ALL][0][ATV_INFRASTRUCTURE_MAINTENANCE] == 0  &&
		maintenance[TT_ALL] == 0  &&
		com_year[0][COST_ALL_CONVOIS] == 0
	);
}


/* most recent savegame version: now with detailed finance statistics by type of transport */
void finance_t::rdwr(loadsave_t *file) {
	/* following lines enables FORWARD compatibility 
	/ you will be still able to load future versions of games with:
	* 	longer history
	*	more transport_types
	*	and new items in ATC_ or ATV_
	*/
	sint8 max_years  = MAX_PLAYER_HISTORY_YEARS2;
	sint8 max_months = MAX_PLAYER_HISTORY_MONTHS2;
	sint8 max_tt     = TT_MAX;
	sint8 max_atc    = ATC_MAX;
	sint8 max_atv    = ATV_MAX;

	// used for reading longer history
	sint64 dummy = 0;

	// calc finance history for TT_ALL to save it correctly
	if( ! file->is_loading() ) { 
		calc_finance_history();
	}

	if( file->get_version() >= 111005 ) { // detailed statistic were introduded in 111005
		file->rdwr_byte( max_years );
		file->rdwr_byte( max_months );
		file->rdwr_byte( max_tt ); // tt = transport type
		file->rdwr_byte( max_atc ); // atc = accounting type common
		file->rdwr_byte( max_atv ); // atv = accounting type vehicles
	
		for(int year = 0;  year < max_years ; ++year ) {
			for( int cost_type = 0; cost_type < max_atc ;  ++cost_type  ) {
				if( ( year < MAX_PLAYER_HISTORY_YEARS ) && ( cost_type < ATC_MAX ) ) {
					file->rdwr_longlong( com_year[year][cost_type] );
				} else {
						file->rdwr_longlong( dummy );
				}
			}
		}
		for(int month = 0; month < max_months; ++month) {
			for( int cost_type = 0; cost_type < max_atc;  ++cost_type ) {
				if( ( month < MAX_PLAYER_HISTORY_MONTHS ) && ( cost_type < ATC_MAX ) ) {
					file->rdwr_longlong( com_month[month][cost_type] );
				} else {
						file->rdwr_longlong( dummy );
				}
			}
		}
		for(int tt=0; tt < max_tt; ++tt){
			for( int year = 0;  year < max_years;  ++year ) {
				for( int cost_type = 0; cost_type < max_atv;  ++cost_type  ) {
					if( ( tt < TT_MAX ) && ( year < MAX_PLAYER_HISTORY_YEARS ) && ( cost_type < ATV_MAX ) ) {
						file->rdwr_longlong( veh_year[tt][year][cost_type] );
					} else {
						file->rdwr_longlong( dummy );
					}
				}
			}
		} 
		for(int tt=0; tt < max_tt; ++tt){
			for( int month = 0; month < max_months; ++month ) {
				for( int cost_type = 0; cost_type < max_atv;  ++cost_type  ) {
					if( ( tt < TT_MAX ) && ( month < MAX_PLAYER_HISTORY_MONTHS ) && ( cost_type < ATV_MAX ) ) {
						file->rdwr_longlong( veh_month[tt][month][cost_type] );
					} else {
						file->rdwr_longlong( dummy );
					}
				}
			}
		} 
	}
}


void finance_t::roll_history_month() {
	// undistinguishable
	for (int i=MAX_PLAYER_HISTORY_MONTHS-1; i>0; i--) {
		for(int accounting_type=0; accounting_type<ATC_MAX; ++accounting_type){
			com_month[i][accounting_type] = com_month[i-1][accounting_type];
		}
	}
	for(int i=0; i<ATC_MAX; ++i){
		if(i != ATC_ALL_CONVOIS){
			com_month[0][i] = 0;
		}
	}
	// vehicles
	for(int tt=0; tt<TT_MAX; ++tt){
		for (int i=MAX_PLAYER_HISTORY_MONTHS-1; i>0; i--) {
			for(int accounting_type=0; accounting_type<ATV_MAX; ++accounting_type){
				veh_month[tt][i][accounting_type] = veh_month[tt][i-1][accounting_type];
			}
		}
		for(int accounting_type=0; accounting_type<ATV_MAX; ++accounting_type){
			veh_month[tt][0][accounting_type] = 0;
		}
	}
}


void finance_t::roll_history_year() {
	// undistinguishable
	for (int i=MAX_PLAYER_HISTORY_YEARS-1; i>0; i--) {
		for(int accounting_type=0; accounting_type<ATC_MAX; ++accounting_type){
			com_year[i][accounting_type] = com_year[i-1][accounting_type];
		}
	}
	for(int i=0; i<ATC_MAX; ++i){
		if(i != ATC_ALL_CONVOIS){
			com_year[0][i] = 0;
		}
	}
	// vehicles
	for(int tt=0; tt<TT_MAX; ++tt){
		for (int i=MAX_PLAYER_HISTORY_YEARS-1; i>0; i--) {
			for(int accounting_type=0; accounting_type<ATV_MAX; ++accounting_type){
				veh_year[tt][i][accounting_type] = veh_year[tt][i-1][accounting_type];
			}
		}
		for(int accounting_type=0; accounting_type<ATV_MAX; ++accounting_type){
			veh_year[tt][0][accounting_type] = 0;
		}
	}
}


int finance_t::translate_index_cost_to_atc(const int cost_index) const
{
	static int cost_to_atc_indices[] = {
		-1,		// COST_CONSTRUCTION
		-1,		// COST_VEHICLE_RUN
		-1,		// COST_NEW_VEHICLE
		-1,		// COST_INCOME
		-1,		// COST_MAINTENANCE
		-1,		// COST_ASSETS
		ATC_CASH,	// COST_CASH - cash can not be assigned to transport type
		ATC_NETWEALTH,	// COST_NETWEALTH -||-
		-1,		// COST_PROFIT
		-1,		// COST_OPERATING_PROFIT
		-1,		// COST_MARGIN
		-1,	        // COST_ALL_TRANSPORTED
		-1,		// ATV_COST_POWERLINES
		-1,		// COST_TRANSPORTED_PAS
		-1,		// COST_TRANSPORTED_MAIL
		-1,		// COST_TRANSPORTED_GOOD
		ATC_ALL_CONVOIS,        // COST_ALL_CONVOIS
		ATC_SCENARIO_COMPLETED, // COST_SCENARIO_COMPLETED,// scenario success (only useful if there is one ... )
		-1,		// COST_WAY_TOLLS,
		ATC_MAX		// MAX_PLAYER_COST
		};

	return (cost_index < MAX_PLAYER_COST) ? cost_to_atc_indices[cost_index] :  -1;
}


// returns -1 or -2 if not found !!
// -1 --> set this value to 0, -2 -->use value from old statistic
int finance_t::translate_index_cost_to_at(int cost_index) {
	static int indices[] = {
		ATV_CONSTRUCTION_COST,  // COST_CONSTRUCTION
		ATV_RUNNING_COST,       // COST_VEHICLE_RUN
		ATV_NEW_VEHICLE,        // COST_NEW_VEHICLE
		ATV_REVENUE_TRANSPORT,  // COST_INCOME
		ATV_INFRASTRUCTURE_MAINTENANCE, // COST_MAINTENANCE
		ATV_NON_FINANTIAL_ASSETS,// COST_ASSETS
		-2,                     // COST_CASH - cash can not be assigned to transport type
		-2,                     // COST_NETWEALTH -||-
		ATV_PROFIT,             // COST_PROFIT
		ATV_OPERATING_PROFIT,   // COST_OPERATING_PROFIT
		ATV_PROFIT_MARGIN,      // COST_MARGIN
		ATV_TRANSPORTED,        // COST_ALL_TRANSPORTED
		-1,                     // ATV_COST_POWERLINES
		ATV_TRANSPORTED_PASSENGER, // COST_TRANSPORTED_PAS
		ATV_TRANSPORTED_MAIL,   // COST_TRANSPORTED_MAIL
		ATV_TRANSPORTED_GOOD,   // COST_TRANSPORTED_GOOD
		-2,                     // COST_ALL_CONVOIS
		-2,                     // COST_SCENARIO_COMPLETED,// scenario success (only useful if there is one ... )
		ATV_WAY_TOLL,           // COST_WAY_TOLLS,
		ATV_MAX                 // MAX_PLAYER_COST
		};

	return (cost_index < MAX_PLAYER_COST) ? indices[cost_index] :  -2;
}


transport_type finance_t::translate_utyp_to_tt(const int utyp) const {
	switch(utyp){
		case haus_besch_t::bahnhof:      return TT_RAILWAY;
		case haus_besch_t::bushalt:      return TT_ROAD;
		case haus_besch_t::hafen:        return TT_SHIP;
		case haus_besch_t::binnenhafen:  return TT_SHIP;
		case haus_besch_t::airport:      return TT_AIR;
		case haus_besch_t::monorailstop: return TT_MONORAIL;
		case haus_besch_t::bahnhof_geb:  return TT_RAILWAY;
		case haus_besch_t::bushalt_geb:  return TT_ROAD;
		case haus_besch_t::hafen_geb:    return TT_SHIP;
		case haus_besch_t::binnenhafen_geb: return TT_SHIP;
		case haus_besch_t::airport_geb:  return TT_AIR;
		case haus_besch_t::monorail_geb: return TT_MONORAIL;
		default: return TT_OTHER;
	}
}


transport_type finance_t::translate_waytype_to_tt(const waytype_t wt) const {
	switch(wt){
		case road_wt:      return TT_ROAD;
		case track_wt:     return TT_RAILWAY;
		case water_wt:     return TT_SHIP;
		case monorail_wt:  return TT_MONORAIL;
		case maglev_wt:    return TT_MAGLEV;
		case tram_wt:      return TT_TRAM;
		case narrowgauge_wt: return TT_NARROWGAUGE;
		case air_wt:       return TT_AIR;
		case powerline_wt: return TT_POWERLINE;
		case ignore_wt:
		case overheadlines_wt:
		default:           return TT_OTHER;
	}
}

