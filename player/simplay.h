/*
 * Copyright (c) 1997 - 2002 Hansjörg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 */

#ifndef simplay_h
#define simplay_h

#include "../dataobj/pwd_hash.h"
#include "../simtypes.h"
#include "../simlinemgmt.h"

#include "../halthandle_t.h"
#include "../convoihandle_t.h"

#include "../dataobj/koord.h"

#include "../tpl/slist_tpl.h"
#include "../tpl/vector_tpl.h"


/** 
 * supersedes COST_ types, that CAN NOT be distinguished by type of transport-
 * - concerning to whole company
 * @author Jan Korbel
 */
enum accounting_type_common {
	ATC_CASH = 0,		// Cash, COST_CASH
	ATC_NETWEALTH,		// Total Cash + Assets, COST_NETWEALTH
	ATC_ALL_CONVOIS,        // COST_ALL_CONVOIS
	ATC_SCENARIO_COMPLETED, // scenario success (only useful if there is one ... ), COST_SCENARIO_COMPLETED
	ATC_INTERESTS,		// Experimental has it -> making savegame format a little bit more compatible between standard and experimental, COST_INTERESTS
	ATC_CREDIT_LIMIT, 	// Experimental, too, COST_CREDIT_LIMITS
	ATC_MAX
};

/** 
 * supersedes COST_ types, that CAN be distinguished by type of transport 
 * @author Jan Korbel
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
	ATV_EXPENDITURE,		// total expenditure = RUNNING_COSTS+VEHICLE_MAINTENANCE+INFRACTRUCTURE_MAINTENANCE 
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

#define MAX_PLAYER_HISTORY_YEARS  (12) // number of years to keep history
#define MAX_PLAYER_HISTORY_MONTHS  (12) // number of months to keep history


class karte_t;
class fabrik_t;
class koord3d;
class werkzeug_t;

/**
 * play info for simutrans human and AI are derived from this class
 */
class spieler_t
{
public:
	enum { MAX_KONTO_VERZUG = 3 };

	enum { EMPTY=0, HUMAN=1, AI_GOODS=2, AI_PASSENGER=3, MAX_AI, PASSWORD_PROTECTED=128 };

protected:
	char spieler_name_buf[256];

	/*
	 * holds total number of all halts, ever built
	 * @author hsiegeln
	 */
	sint32 haltcount;

	/**
	 * finance history - will supersede the finance_history by hsiegeln 
	 * from version 111
	 * containes values having relation with whole company but not with particular
	 * type of transport (com - common)
	 * @author Jan Korbel
	 */
	sint64 finance_history_com_year[MAX_PLAYER_HISTORY_YEARS][ATC_MAX];
	sint64 finance_history_com_month[MAX_PLAYER_HISTORY_MONTHS][ATC_MAX];

	/**
	 * finance history having relation with particular type of service
	 * @author Jan Korbel
	 */
	sint64 finance_history_veh_year[TT_MAX][MAX_PLAYER_HISTORY_YEARS][ATV_MAX];
	sint64 finance_history_veh_month[TT_MAX][MAX_PLAYER_HISTORY_MONTHS][ATV_MAX];

	/**
	* Finance History - will supercede the finances by Owen Rudge
	* Will hold finances for the most recent 12 years
	* @author hsiegeln
	*/
	sint64 finance_history_year[MAX_PLAYER_HISTORY_YEARS][MAX_PLAYER_COST];
	sint64 finance_history_month[MAX_PLAYER_HISTORY_MONTHS][MAX_PLAYER_COST];

	/**
	 * Monthly maintenance cost
	 * @author Hj. Malthaner
	 */
	sint32 maintenance[TT_MAX];

	/**
	 * monthly vehicle maintenance cost
	 * author Jan Korbel
	 */
	sint32 vehicle_maintenance[TT_MAX];

	/**
	 * Die Welt in der gespielt wird.
	 *
	 * @author Hj. Malthaner
	 */
	static karte_t *welt;

	/**
	 * Der Kontostand.
	 *
	 * @author Hj. Malthaner
	 */
	sint64 konto;

	// remember the starting money
	sint64 starting_money;

	/**
	 * Zählt wie viele Monate das Konto schon ueberzogen ist
	 *
	 * @author Hj. Malthaner
	 */
	sint32 konto_ueberzogen;

	slist_tpl<halthandle_t> halt_list; ///< Liste der Haltestellen

	class income_message_t {
	public:
		char str[33];
		koord pos;
		sint32 amount;
		sint8 alter;
		income_message_t() { str[0]=0; alter=127; pos=koord::invalid; amount=0; }
		income_message_t( sint32 betrag, koord pos );
		void * operator new(size_t s);
		void operator delete(void *p);
	};

	slist_tpl<income_message_t *>messages;

	void add_message(koord k, sint32 summe);

	/**
	 * Kennfarbe (Fahrzeuge, Gebäude) des Speielers
	 * @author Hj. Malthaner
	 */
	uint8 kennfarbe1, kennfarbe2;

	/**
	 * Player number; only player 0 can do interaction
	 * @author Hj. Malthaner
	 */
	uint8 player_nr;

	/**
	 * Adds somme amount to the maintenance costs
	 * @param change the change
	 * @return the new maintenance costs
	 * @author Hj. Malthaner
	 */
	sint32 add_maintenance(sint32 change, waytype_t const wt=ignore_wt);

	/**
	 * Ist dieser Spieler ein automatischer Spieler?
	 * @author Hj. Malthaner
	 */
	bool automat;

	/**
	 * Are this player allowed to do any changes?
	 * @author Hj. Malthaner
	 */
	bool locked;

	bool unlock_pending;

	// contains the password hash for local games
	pwd_hash_t pwd_hash;

	/**
	 * Translates finance statistisc from new format to old (version<=111) one.
	 * Used for saving data in old format
	 * @author Jan Korbel
	 */
	void translate_at_to_cost();

	/**
	 * Translates finance statistisc from old (version<=111) format to new one.
	 * Used for loading data from old format
	 * @author Jan Korbel
	 */
	void translate_cost_to_at();

	int translate_index_cost_to_at(int a);

	/**
	 * Translates waytype_t to transport_type
	 * @author Jan Korbel
	 */
	transport_type translate_waytype_to_tt(const waytype_t wt) const;

public:
	/**
	 * sums up "count" with number of convois in statistics, 
	 * supersedes buche( count, COST_ALL_CONVOIS)
	 * @author Jan Korbel
	 */
	void add_convoi_number(int count){
		finance_history_com_year[0][ATC_ALL_CONVOIS] += count;
		finance_history_com_month[0][ATC_ALL_CONVOIS] += count;
	}

	/**
	 * Adds construction costs to accounting statistics, 
	 * @param amount How much does it cost
	 * @param tt type of transport
	 * @author Jan Korbel
	 */
	void add_construction_costs(const sint64 amount, const koord k, const waytype_t wt=ignore_wt);

	static void add_construction_costs(spieler_t * const sp, const sint64 amount, const koord k, const waytype_t wt=ignore_wt);

	/*
	 * displayes amount of money when koordinates and on screen
	 * reworked function buche()
	 */
	void add_money_message(const sint64 amount, const koord k);

	/**
	 * Accounts bought/sold vehicles
	 * @param price money used for purchase of vehicle,
	 *              negative value = vehicle bought,
	 *              negative value = vehicle sold
	 * @param tt type of transport for accounting purpose
	 * @author Jan Korbel
	 */
	void add_new_vehicle(const sint64 price, const koord k, const waytype_t wt=ignore_wt);

	/**
	 * Adds income to accounting statistics. Cargo with unknown cathegory will be 
	 * accounted as cathegory 7. 
	 * @param amount earned money
	 * @param tt transport type used in accounting statistics
	 * @param cathegory parameter "catg" of goods [0,7] from pak files, special 
	 *                  values -2 for passenger and -1 for mail
	 *                  if parameter tt is TT_POWERLINE, cathegory is ignored
	 * @author Jan Korbel
	 */
	void add_revenue(const sint64 amount, const koord k, const waytype_t wt=ignore_wt, sint32 cathegory=2);

	/**
         * Adds running costs to accounting statistics. 
         * this function is called very often --> inline
         * @param amount How much does it cost
         * @param wt
	 * @author Jan Korbel
         */
        void add_running_costs(const sint64 amount, const waytype_t wt=ignore_wt);

	/**
	 * books toll payed by our company to someone else
	 * @param amount money payed to our company
	 * @param tt type of transport used for assounting statistisc
	 * @author Jan Korbel
	 */
	void add_toll_payed(const sint64 amount, const waytype_t wt=ignore_wt);

	/**
	 * books toll payed to out company by someone else
	 * @param amount money payed for usage of our roads,railway,channels, ... ; positive sign
	 * @param tt type of transport used for assounting statistisc
	 * @author Jan Korbel
	 */
	void add_toll_received(const sint64 amount, waytype_t wt=ignore_wt);

	/**
	 * Add amount of transported passanger, mail, goods to accounting statistics
	 * @param amount number of transported units
	 * @papam tt type of transport
	 * @param cathegory constegory of transported items (-2 passanger, -1 mail, 
	 *                  other same as in the pak files)
	 * @author Jan Korbel
	 */
	void add_transported(const sint64 amount, const waytype_t wt=ignore_wt, int index=2);

	virtual bool set_active( bool b ) { return automat = b; }

	bool is_active() const { return automat; }

	bool is_locked() const { return locked; }

	bool is_unlock_pending() const { return unlock_pending; }

	void unlock(bool unlock_, bool unlock_pending_=false) { locked = !unlock_; unlock_pending = unlock_pending_; }

	void check_unlock( const pwd_hash_t& hash ) { locked = (pwd_hash != hash); }

	// some routine needs this for direct manipulation
	pwd_hash_t& access_password_hash() { return pwd_hash; }

	// this type of AIs identifier
	virtual uint8 get_ai_id() const { return HUMAN; }

	// @author hsiegeln
	simlinemgmt_t simlinemgmt;

	/**
	 * Age messages (move them upwards)
	 * @author Hj. Malthaner
	 */
	void age_messages(long delta_t);

	/* Handles player colors ...
	* @author prissi
	*/
	uint8 get_player_color1() const { return kennfarbe1; }
	uint8 get_player_color2() const { return kennfarbe2; }
	void set_player_color(uint8 col1, uint8 col2);

	/**
	 * Name of the player
	 * @author player
	 */
	const char* get_name() const;
	void set_name(const char *);

	sint8 get_player_nr() const {return player_nr; }

	/**
	 * return true, if the owner is none, myself or player(1), i.e. the ownership can be taken by player test
	 * @author prissi
	 */
	static bool check_owner( const spieler_t *owner, const spieler_t *test );

	/**
	 * @param welt Die Welt (Karte) des Spiels
	 * @param color Kennfarbe des Spielers
	 * @author Hj. Malthaner
	 */
	spieler_t(karte_t *welt, uint8 player_nr );

	virtual ~spieler_t();

	sint32 get_maintenance(transport_type tt=TT_ALL) const { assert(tt<TT_MAX); return maintenance[tt]; }
	
	/**
	 * returns maintenance with bits_per_month
	 * @author Jan Korbel
	 */
	sint64 get_maintenance_with_bits(transport_type tt=TT_ALL) const;

	/**
	 * returns vehicle maintenance with bits_per_month
	 * @author Jan Korbel
	 */
	sint64 get_vehicle_maintenance_with_bits(transport_type tt=TT_ALL) const;

	static sint32 add_maintenance(spieler_t *sp, sint32 const change, waytype_t const wt=ignore_wt) {
		if(sp) {
			return sp->add_maintenance(change, wt);
		}
		return 0;
	}

	// Owen Rudge, finances
	void buche(sint64 betrag, koord k, player_cost type);

	// do the internal accounting (currently only used externally for running costs of convois)
	void buche(sint64 betrag, player_cost type);

	// this is also save to be called with sp==NULL, which may happen for unowned objects like bridges, ways, trees, ...
	static void accounting(spieler_t* sp, sint64 betrag, koord k, player_cost pc);

	/**
	 * @return Kontostand als double (Gleitkomma) Wert
	 * @author Hj. Malthaner
	 */
	double get_konto_als_double() const { return konto / 100.0; }

	/**
	 * @return true wenn Konto Überzogen ist
	 * @author Hj. Malthaner
	 */
	int get_konto_ueberzogen() const { return konto_ueberzogen; }

	/**
	 * Zeigt Meldungen aus der Queue des Spielers auf dem Bildschirm an
	 * @author Hj. Malthaner
	 */
	void display_messages();

	/**
	 * Wird von welt in kurzen abständen aufgerufen
	 * @author Hj. Malthaner
	 */
	virtual void step();

	/**
	 * Wird von welt nach jedem monat aufgerufen
	 * @author Hj. Malthaner
	 */
	virtual void neuer_monat();

	/**
	 * Methode fuer jaehrliche Aktionen
	 * @author Hj. Malthaner
	 */
	virtual void neues_jahr() {}

	/**
	 * Erzeugt eine neue Haltestelle des Spielers an Position pos
	 * @author Hj. Malthaner
	 */
	halthandle_t halt_add(koord pos);

	/**
	 * needed to transfer ownership
	 * @author prissi
	 */
	void halt_add(halthandle_t h);

	/**
	 * Entfernt eine Haltestelle des Spielers aus der Liste
	 * @author Hj. Malthaner
	 */
	void halt_remove(halthandle_t halt);

	/**
	 * Gets haltcount, for naming purposes
	 * @author hsiegeln
	 */
	int get_haltcount() const { return haltcount; }

	/**
	 * Lädt oder speichert Zustand des Spielers
	 * @param file die offene Save-Datei
	 * @author Hj. Malthaner
	 */
	virtual void rdwr(loadsave_t *file);

	/*
	 * called after game is fully loaded;
	 */
	virtual void laden_abschliessen();

	virtual void rotate90( const sint16 y_size );

	/**
	* Returns the finance history for player
	* @author hsiegeln
	*/
	sint64 get_finance_history_year(int year, int type) { return finance_history_year[year][type]; }
	sint64 get_finance_history_month(int month, int type) { return finance_history_month[month][type]; }

	/**
	* Returns the finance history for player
	* @author hsiegeln, jk271
	* 'proxy' for more complicated internal data structures
	* int tt is COST_ !!!
	*/
	sint64 get_finance_history_year(int tt, int year, int type);
	sint64 get_finance_history_month(int tt, int month, int type);

	/**
	* Returns the finance history (indistinguishable part) for player
	* @author hsiegeln, Jan Korbel
	*/
	sint64 get_finance_history_com_year(int year, int type) { return finance_history_com_year[year][type]; }
	sint64 get_finance_history_com_month(int month, int type) { return finance_history_com_month[month][type]; }

	/**
	* Returns the finance history (distinguishable by type of transport) for player
	* @author hsiegeln, Jan Korbel
	*/
	sint64 get_finance_history_veh_year(transport_type tt, int year, int type) { return finance_history_veh_year[tt][year][type]; }
	sint64 get_finance_history_veh_month(transport_type tt, int month, int type) { return finance_history_veh_month[tt][month][type]; }

	/**
	 * Returns pointer to finance history for player
	 * @author hsiegeln
	 */
	sint64* get_finance_history_year() { return *finance_history_year; }
	sint64* get_finance_history_month() { return *finance_history_month; }

	/**
	 * @return finance history of indistinguishable (by type of transport) 
	 * part of finance statistics
	 * @author Jan Korbel
	 */
	sint64* get_finance_history_com_year() { return *finance_history_com_year; }
	sint64* get_finance_history_com_month() { return *finance_history_com_month; }

	/**
	 * @return finance history for vehicles
	 * @author Jan Korbel
	 */
	sint64* get_finance_history_veh_year(transport_type tt) { assert(tt<TT_MAX_VEH); return *finance_history_veh_year[tt]; }
	sint64* get_finance_history_veh_month(transport_type tt) { assert(tt<TT_MAX_VEH); return *finance_history_veh_month[tt]; }

	/**
	* Returns the world the player is in
	* @author hsiegeln
	*/
	static karte_t *get_welt() { return welt; }

	/**
	* Calculates the finance history for player
	* @author hsiegeln
	*/
	void calc_finance_history();

	/**
	* Calculates the assets of the player
	*/
	void calc_assets();

	/**
	* Updates the assets value of the player
	*/
	void update_assets(sint64 const delta);

	/**
	* rolls the finance history for player (needed when neues_jahr() or neuer_monat()) triggered
	* @author hsiegeln
	*/
	void roll_finance_history_year();
	void roll_finance_history_month();

	/**
	 * Rückruf, um uns zu informieren, dass ein Vehikel ein Problem hat
	 * @author Hansjörg Malthaner
	 * @date 26-Nov-2001
	 */
	virtual void bescheid_vehikel_problem(convoihandle_t cnv,const koord3d ziel);

	/**
	 * Tells the player the result of tool-work commands
	 * If player is active then play sound, popup error msg etc
	 * AI players react upon this call and proceed
	 * local is true if tool was called by player on our client
	 * @author Dwachs
	 */
	virtual void tell_tool_result(werkzeug_t *tool, koord3d pos, const char *err, bool local);

	/**
	 * Tells the player that the factory
	 * is going to be deleted (flag==0)
	 * Bernd Gabriel, Dwachs
	 */
	enum notification_factory_t {
		notify_delete	// notified immediately before object is deleted (and before nulled in the slist_tpl<>)!
	};
	virtual void notify_factory(notification_factory_t, const fabrik_t*) {}

private:
	/* undo informations *
	 * @author prissi
	 */
	vector_tpl<koord3d> last_built;
	waytype_t undo_type;

public:
	void init_undo(waytype_t t, unsigned short max );
	void add_undo(koord3d k);
	sint64 undo();

	// headquarter stuff
private:
	sint32 headquarter_level;
	koord headquarter_pos;

public:
	void add_headquarter(short hq_level, koord hq_pos)
	{
		headquarter_level = hq_level;
		headquarter_pos = hq_pos;
	}
	koord get_headquarter_pos(void) const { return headquarter_pos; }
	short get_headquarter_level(void) const { return headquarter_level; }

	void ai_bankrupt();
};

#endif
