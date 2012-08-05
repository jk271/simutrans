/*
 * finance.h
 *
 * Copyright (c) 1997 - 2012 Hansjörg Malthaner and
 *        the simutrans development team
 *
 * This file is part of the Simutrans project under the artistic license.
 * (see license.txt)
 */

#ifndef finance_h
#define finance_h

#include <assert.h>

#include "../simtypes.h"

// synchronize it with finance.h !!
#define MAX_PLAYER_HISTORY_YEARS  (12) // number of years to keep history
#define MAX_PLAYER_HISTORY_MONTHS  (12) // number of months to keep history

/** this HAS TO be greater or equal than MAX_PLAYER_HIRSTORY_YEARS in file simplay.h !!!!! 
*/
#define MAX_PLAYER_HISTORY_YEARS2  (25) // number of years to keep history

/** this HAS TO be greater or equal than MAX_PLAYER_HIRSTORY_MONTHS in file simplay.h !!!!! 
*/
#define MAX_PLAYER_HISTORY_MONTHS2  (25) // number of months to keep history


/** 
 * supersedes COST_ types, that CAN NOT be distinguished by type of transport-
 * - concerning to whole company
 * @author jk271
 */
enum accounting_type_common {
	ATC_CASH = 0,		// Cash, COST_CASH
	ATC_NETWEALTH,		// Total Cash + Assets, COST_NETWEALTH
	ATC_ALL_CONVOIS,        // COST_ALL_CONVOIS; good for ?? what ??
	ATC_SCENARIO_COMPLETED, // scenario success (only useful if there is one ... ), COST_SCENARIO_COMPLETED
	ATC_INTERESTS,		// Experimental has it -> making savegame format a little bit more compatible between standard and experimental, COST_INTERESTS
	ATC_CREDIT_LIMIT, 	// Experimental, too, COST_CREDIT_LIMITS
	ATC_TAX,		// for future use, at least planed in exp.
	ATC_ROA,		// return on assets = total revenue / all assets ; ( all assets = non-finantial assets + cash); for future use
	ATC_ROE,		// return on equity = total tevenue / (all assets - debths); for future use
	ATC_MAX
};


/** 
 * supersedes COST_ types, that CAN be distinguished by type of transport 
 * @author jk271
 */
enum accounting_type_vehicles {
	// revenue by freight type http://simutrans-germany.com/wiki/wiki/tiki-index.php?page=en_GoodsDef
	ATV_REVENUE_PASSENGER=0, // revenue from passenger transport
	ATV_REVENUE_MAIL,       // revenue from mail transport
	ATV_REVENUE_GOOD,          // revenue from good transport
	ATV_REVENUE_TRANSPORT,	// operating profit = passenger + mail+ goods = COST_INCOME
	ATV_TOLL_RECEIVED,	// toll payed to you by another player
	ATV_REVENUE,            // operating profit = revenue_transport + toll = passenger + mail+ goods + toll_received

	ATV_RUNNING_COST,               // distance based running costs, COST_VEHICLE_RUN
	ATV_VEHICLE_MAINTENANCE,        // monthly vehicle maintenance
	ATV_INFRASTRUCTURE_MAINTENANCE,	// infrastructure maintenance (roads, railway, ...), COST_MAINTENENCE
	ATV_TOLL_PAYED,			// toll payed by you to another player
	ATV_EXPENDITURE,		// total expenditure = RUNNING_COSTS+VEHICLE_MAINTENANCE+INFRACTRUCTURE_MAINTENANCE+TOLL_PAYED
	ATV_OPERATING_PROFIT,		// = AT_REVENUE - AT_EXPENDITURE, COST_OPERATING_PROFIT
	ATV_NEW_VEHICLE,			// New vehicles
	ATV_CONSTRUCTION_COST,		// costruction cost, COST_COSTRUCTION mapped here
	ATV_PROFIT,			// = AT_OPERATING_PROFIT - (COSTRUCTION_COST + NEW_VEHICLE)(and INTERESTS in Experimental), COST_PROFIT
	ATV_WAY_TOLL,			// = ATV_TOLL_PAYED + ATV_TOLL_RECEIVED; ATV_WAY_TOLL = COST_WAY_TOLLS
	ATV_NON_FINANTIAL_ASSETS,	// value of vehicles owned by your company, COST_ASSETS
	ATV_PROFIT_MARGIN,		// AT_OPERATING_PROFIT / AT_REVENUE, COST_MARGIN

	
	ATV_TRANSPORTED_PASSENGER, // numer of transported passanger, COST_TRANSPORTED_PAS
	ATV_TRANSPORTED_MAIL,      // COST_TRANSPORTED_MAIL
	ATV_TRANSPORTED_GOOD,         // COST_TRANSPORTED_GOOD mapped here, all ATV_TRANSPORTED_* mapped to COST_TRANSPORTED_GOOD
	ATV_TRANSPORTED,           // COST_ALL_TRANSPORTED mapped here

	ATV_MAX
};


enum player_cost {
	COST_CONSTRUCTION=0,// Construction
	COST_VEHICLE_RUN,   // Vehicle running costs
	COST_NEW_VEHICLE,   // New vehicles
	COST_INCOME,        // Income
	COST_MAINTENANCE,   // Upkeep
	COST_ASSETS,        // value of all vehicles and buildings
	COST_CASH,          // Cash
	COST_NETWEALTH,     // Total Cash + Assets
	COST_PROFIT,        // COST_POWERLINES+COST_INCOME-(COST_CONSTRUCTION+COST_VEHICLE_RUN+COST_NEW_VEHICLE+COST_MAINTENANCE)
	COST_OPERATING_PROFIT, // COST_POWERLINES+COST_INCOME-(COST_VEHICLE_RUN+COST_MAINTENANCE)
	COST_MARGIN,        // COST_OPERATING_PROFIT/COST_INCOME
	COST_ALL_TRANSPORTED, // all transported goods
	COST_POWERLINES,	  // revenue from the power grid
	COST_TRANSPORTED_PAS,	// number of passengers that actually reached destination
	COST_TRANSPORTED_MAIL,
	COST_TRANSPORTED_GOOD,
	COST_ALL_CONVOIS,		// number of convois
	COST_SCENARIO_COMPLETED,// scenario success (only useful if there is one ... )
	COST_WAY_TOLLS,
	MAX_PLAYER_COST
};


class loadsave_t;
class karte_t;
class spieler_t;
class scenario_t;


/**
 * Encapsulate margin calculation  (Operating_Profit / Income)
 * @author Ben Love
 */
inline sint64 calc_margin(sint64 operating_profit, sint64 proceeds)
{
	if (proceeds == 0) {
		return 0;
	}
	return (10000 * operating_profit) / proceeds;
}


/**
 * convert to displayed value
 */
inline sint64 convert_money(sint64 value) { return (value + 50) / 100; }


/*
 * finance_history since version around 111.5 
 * I hope that having all finance in one class is better 
 * than having it in more places in spieler_t 
 * Another benefit: It leads to shorter variable names. 
 **/
class finance_t {
	spieler_t * player;

	karte_t * world;

public:
	/**
 	* Der Kontostand.
 	*
 	* @author Hj. Malthaner
 	*/
	sint64 konto;

	/**
	 * Zählt wie viele Monate das Konto schon ueberzogen ist
	 *
	 * @author Hj. Malthaner
	 */
	sint32 konto_ueberzogen;

	// remember the starting money
	sint64 starting_money;

private:
	/**
	 * finance history - will supersede the finance_history by hsiegeln 
	 * from version 111 or 112
	 * containes values having relation with whole company but not with particular
	 * type of transport (com - common)
 	* @author jk271
 	*/
	sint64 com_year[MAX_PLAYER_HISTORY_YEARS2][ATC_MAX];
	sint64 com_month[MAX_PLAYER_HISTORY_MONTHS2][ATC_MAX];

	/**
 	* finance history having relation with particular type of service
 	* @author jk271
 	*/
	sint64 veh_year[TT_MAX][MAX_PLAYER_HISTORY_YEARS2][ATV_MAX];
	sint64 veh_month[TT_MAX][MAX_PLAYER_HISTORY_MONTHS2][ATV_MAX];

	/**
 	* Monthly maintenance cost
 	* @author Hj. Malthaner
 	*/
	sint32 maintenance[TT_MAX];

	/**
 	* monthly vehicle maintenance cost
 	* @author jk271
 	*/
	sint32 vehicle_maintenance[TT_MAX];

public:
	finance_t(spieler_t * _player, karte_t * _world);


	inline void book_construction_costs(const sint64 amount, const waytype_t wt, const int utyp){
		transport_type tt = translate_waytype_to_tt(wt);
		if(( tt == TT_OTHER ) && ( utyp !=0 ) ) {
			tt = translate_utyp_to_tt(utyp);
		}
		veh_year[tt][0][ATV_CONSTRUCTION_COST] += (sint64) amount;
		veh_month[tt][0][ATV_CONSTRUCTION_COST] += (sint64) amount;

		konto += amount;
	}


	/**
	 * sums up "count" with number of convois in statistics, 
	 * supersedes buche( count, COST_ALL_CONVOIS)
	 * @author jk271
	 */
	inline void book_convoi_number( const int count ) {
		com_year[0][ATC_ALL_CONVOIS] += count;
		com_month[0][ATC_ALL_CONVOIS] += count;
	}


	inline sint32 book_maintenance(sint32 change, waytype_t const wt, const int utyp) {
		transport_type tt = translate_waytype_to_tt(wt);
		if(( tt ==TT_OTHER ) && ( utyp !=0 ) ) {
			tt = translate_utyp_to_tt(utyp);
		}
		assert(tt!=TT_ALL);
		maintenance[tt] += change;
		maintenance[TT_ALL] += change;
		return maintenance[tt];
	}


	inline void book_new_vehicle(const sint64 amount, const waytype_t wt){
		const transport_type tt = translate_waytype_to_tt(wt);

		veh_year[ tt][0][ATV_NEW_VEHICLE] += (sint64) amount;
		veh_month[tt][0][ATV_NEW_VEHICLE] += (sint64) amount;
		veh_year[ tt][0][ATV_NON_FINANTIAL_ASSETS] -= (sint64) amount;
		veh_month[tt][0][ATV_NON_FINANTIAL_ASSETS] -= (sint64) amount;

		konto += amount;
	}

	inline void book_revenue(const sint64 amount, const waytype_t wt, sint32 index){
		const transport_type tt = translate_waytype_to_tt(wt);

		index = ((0 <= index) && (index <= 2)? index : 2);

		veh_year[tt][0][ATV_REVENUE_PASSENGER+index] += (sint64) amount;
		veh_month[tt][0][ATV_REVENUE_PASSENGER+index] += (sint64) amount;

		konto += amount;
	}

	inline void book_running_costs(const sint64 amount, const waytype_t wt){
		const transport_type tt = translate_waytype_to_tt(wt);
		veh_year[tt][0][ATV_RUNNING_COST] += amount;
		veh_month[tt][0][ATV_RUNNING_COST] += amount;
		konto += amount;
	}

	inline void book_toll_payed(const sint64 amount, const waytype_t wt){
		const transport_type tt =  translate_waytype_to_tt(wt);
		veh_year[tt][0][ATV_TOLL_PAYED] += (sint64) amount;
		veh_month[tt][0][ATV_TOLL_PAYED] += (sint64) amount;
		konto += amount;
	}

	inline void book_toll_received(const sint64 amount, const waytype_t wt){
		const transport_type tt = translate_waytype_to_tt(wt);
		veh_year[tt][0][ATV_TOLL_RECEIVED] += (sint64) amount;
		veh_month[tt][0][ATV_TOLL_RECEIVED] += (sint64) amount;
		konto += amount;
	}

	inline void book_transported(const sint64 amount, const waytype_t wt, int index){
		const transport_type tt = translate_waytype_to_tt(wt);

		// there are: passenger, mail, goods
		if( (index < 0) || (index > 2)){
			index = 2;
		}

		veh_year[ tt][0][ATV_TRANSPORTED_PASSENGER+index] += amount;
		veh_month[tt][0][ATV_TRANSPORTED_PASSENGER+index] += amount;
	}

	/**
	* Calculates the finance history for player
	* @author hsiegeln
	*/
	void calc_finance_history();

	/* workaround, used for charts in money_frame */
	void calc_flat_view_month(int tt, sint64 (&flat_view_month)[MAX_PLAYER_HISTORY_MONTHS][MAX_PLAYER_COST]);
	void calc_flat_view_year( int tt, sint64 (&flat_view_year)[ MAX_PLAYER_HISTORY_YEARS ][MAX_PLAYER_COST]);

	/**
 	* Translates finance statistisc from new format to old (version<=111) one.
 	* Used for saving data in old format
 	* @author jk271
 	*/
	void export_to_cost_month(sint64 (&finance_history_month)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]);
	void export_to_cost_year( sint64 (&finance_history_year)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]);

	/**
	 * Returns amount of money on account (also known as konto)
	 */
	inline sint64 get_account_balance() { return konto; }

	/**
	* Returns the finance history for player
	* @author hsiegeln, jk271
	* 'proxy' for more complicated internal data structures
	* int tt is COST_ !!!
	*/
	sint64 get_finance_history_year(int tt, int year, int type);
	sint64 get_finance_history_month(int tt, int month, int type);

	/**
	 * used in scripted scenario
	 */
	sint64 get_finance_history_month_converted( int month, int type);

	/**
	* Returns the finance history (indistinguishable part) for player
	* @author hsiegeln, jk271
	*/
	sint64 get_finance_history_com_year(int year, int type) { return com_year[year][type]; }
	sint64 get_finance_history_com_month(int month, int type) { return com_month[month][type]; }

	/**
	* Returns the finance history (distinguishable by type of transport) for player
	* @author hsiegeln, jk271
	*/
	sint64 get_finance_history_veh_year(transport_type tt, int year, int type) { return veh_year[tt][year][type]; }
	sint64 get_finance_history_veh_month(transport_type tt, int month, int type) { return veh_month[tt][month][type]; }

	/**
 	* @return finance history of indistinguishable (by type of transport) 
 	* part of finance statistics
 	* @author jk271
 	*/
	sint64* get_finance_history_com_year() { return *com_year; }
	sint64* get_finance_history_com_month() { return *com_month; }

	/**
 	* @return finance history for vehicles
 	* @author jk271
 	*/
	sint64* get_finance_history_veh_year(transport_type tt) { assert(tt<TT_MAX_VEH); return *veh_year[tt]; }
	sint64* get_finance_history_veh_month(transport_type tt) { assert(tt<TT_MAX_VEH); return *veh_month[tt]; }

	/**
	 * returns maintenance 
	 * @param tt transport type (Truck, Ship Air, ...)
	 */
	sint32 get_maintenance(transport_type tt=TT_ALL) const { assert(tt<TT_MAX); return maintenance[tt]; }

	/**
	 * returns maintenance with bits_per_month
	 * @author jk271
	 */
	sint64 get_maintenance_with_bits(transport_type tt=TT_ALL) const;
	
	inline sint64 get_netwealth() const { return com_year[0][ATC_NETWEALTH]; }

	inline sint64 get_scenario_completed() const { return com_year[0][ATC_SCENARIO_COMPLETED]; }

	/**
	 * returns vehicle maintenance with bits_per_month
	 * @author jk271
	 */
	sint64 get_vehicle_maintenance_with_bits(transport_type tt=TT_ALL) const;

	/**
	 * returns TRUE if (account(=konto) + assets )>0
	 */
	bool has_money_or_assets() { return (( konto + get_finance_history_veh_year(TT_ALL, 0, ATV_NON_FINANTIAL_ASSETS) ) > 0 ); }

	/**
 	* Translates finance statistisc from old (version<=111) format to new one.
 	* Used for loading data from old format
 	* @author jk271
 	*/
	void import_from_cost_month(const sint64 (& finance_history_month)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]);
	void import_from_cost_year( const sint64 (& finance_history_year)[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST]);

	/**
	* returns true if company bancrupted
	*/
	bool is_bancrupted() const;

	void new_month();

	/**
	* rolls the finance history for player (needed when neues_jahr() or neuer_monat()) triggered
	* @author hsiegeln, jk271
	*/
	void roll_history_year();
	void roll_history_month();

	/* loads or saves finance statistic */
	void rdwr(loadsave_t *file);

	void set_assets(const sint64 (&assets)[TT_MAX]);

	int translate_index_cost_to_at(int cost_);

	int translate_index_cost_to_atc( const int cost_index ) const;

	/**
 	* Translates haus_besch_t to transport_type
	* Building can be assigned to transport type using utyp
 	* @author jk271
 	*/
	transport_type translate_utyp_to_tt(const int utyp) const;

	/**
 	* Translates waytype_t to transport_type
 	* @author jk271
 	*/
	transport_type translate_waytype_to_tt(const waytype_t wt) const;

	void update_assets(sint64 const delta, const waytype_t wt);
};

#endif
