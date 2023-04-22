#include "brand.h"
#include <unordered_set>
#include <unordered_map>
#include <stdarg.h>

namespace brand {

	struct particleData {
		game_object_script particle = {};
		float creationTime = 0;
	};

	struct rDamageData {
		float damage = 0;
		int shots = 0;
		bool kills = false;
	};

	struct eBounceTarget {
		game_object_script target{};
		bool extraRange = 0;
		int priority = 0;
		float distance = 0;
	};

	struct stasisStruct {
		float stasisTime = 0;
		float stasisStart = 0;
		float stasisEnd = 0;
	};

	struct buffList {
		float godBuff = 0;
		float noKillBuff = 0;
		stasisStruct stasis = {};
	};

	struct particleStruct {
		game_object_script obj = {};
		game_object_script target = {};
		game_object_script owner = {};
		float time = 0;
		float castTime = 0;
		vector castingPos = vector::zero;
		bool isZed = false;
		bool isTeleport = false;
	};

	std::vector<game_object_script> targets;
	std::vector<particleData> particleList;
	std::vector<eBounceTarget> eBounceTargets;
	std::vector<particleStruct> particlePredList;

	std::unordered_map<uint32_t, prediction_output> qPredictionList;
	std::unordered_map<uint32_t, prediction_output> wPredictionList;
	std::unordered_map<uint32_t, int> priorityList;
	std::unordered_map<uint32_t, stasisStruct> stasisInfo;
	std::unordered_map<uint32_t, float> stunTime;
	std::unordered_map<uint32_t, float> guardianReviveTime;
	std::unordered_map<uint32_t, float> deathAnimTime;
	std::unordered_map<uint32_t, float> godBuffTime;
	std::unordered_map<uint32_t, float> noKillBuffTime;
	std::unordered_map<uint32_t, float> qDamageList;
	std::unordered_map<uint32_t, float> wDamageList;
	std::unordered_map<uint32_t, float> eDamageList;
	std::unordered_map<uint32_t, rDamageData> rDamageList;

	static constexpr uint32_t godBuffList[]
	{
		buff_hash("KayleR"),
		buff_hash("TaricR"),
		buff_hash("SivirE"),
		buff_hash("FioraW"),
		buff_hash("NocturneShroudofDarkness"),
		buff_hash("kindredrnodeathbuff"),
		buff_hash("XinZhaoRRangedImmunity"),
		buff_hash("PantheonE")
	};

	static constexpr uint32_t noKillBuffList[]
	{
		buff_hash("UndyingRage"),
		buff_hash("ChronoShift")
	};

	static constexpr uint32_t stasisBuffList[]
	{
		buff_hash("ChronoRevive"),
		buff_hash("BardRStasis"),
		buff_hash("ZhonyasRingShield"),
		buff_hash("LissandraRSelf")
	};

	static constexpr uint32_t immuneSpells[]
	{
		spell_hash("EvelynnR"),
		spell_hash("ZedR"),
		spell_hash("EkkoR"),
		spell_hash("FizzE"),
		spell_hash("FizzETwo"),
		spell_hash("FizzEBuffer"),
		spell_hash("XayahR"),
		spell_hash("VladimirSanguinePool")
	};

	static constexpr uint32_t ignoreSpells[]
	{
		spell_hash("NunuW"),
		spell_hash("SionR")
	};

	static constexpr float qMana[] { 50, 50, 50, 50, 50 };
	static constexpr float wMana[] { 60, 70, 80, 90, 100 };
	static constexpr float eMana[] { 70, 75, 80, 85, 90 };
	static constexpr float rMana[] { 100, 100, 100 };

	game_object_script bestETarget;

	script_spell* q;
	script_spell* w;
	script_spell* e;
	script_spell* r;

	TreeTab* mainMenu;
	namespace settings {
		namespace draws {
			TreeEntry* wRadius;
			TreeEntry* rDamage;
			TreeEntry* rDamageText;
			TreeEntry* particlePos;
			TreeEntry* stasisPos;
			namespace spellRanges {
				TreeEntry* qRange;
				TreeEntry* wRange;
				TreeEntry* eRange;
				TreeEntry* eExtraRange;
				TreeEntry* rRange;
				TreeEntry* legCircles;
			}
		}
		namespace combo {
			TreeEntry* qCombo;
			TreeEntry* wCombo;
			TreeEntry* eCombo;
			TreeEntry* eExtraCombo;
			TreeEntry* rCombo;
			TreeEntry* rComboLogic;
			TreeEntry* rComboAoE;
			TreeEntry* rComboAoEAmount;
			TreeEntry* rComboKills;
			TreeEntry* rComboMinionBounce;
			TreeEntry* rComboBounces;
		}
		namespace harass {
			TreeEntry* qHarass;
			TreeEntry* wHarass;
			TreeEntry* eHarass;
			TreeEntry* eExtraHarass;
		}
		namespace automatic {
			TreeEntry* towerCheck;
			TreeEntry* attackCheck;
			TreeEntry* fowPred;
			TreeEntry* qeLogic;
			TreeEntry* qStun;
			TreeEntry* wStun;
			TreeEntry* qDash;
			TreeEntry* wDash;
			TreeEntry* qCast;
			TreeEntry* wCast;
			TreeEntry* qChannel;
			TreeEntry* wChannel;
			TreeEntry* qStasis;
			TreeEntry* wStasis;
			TreeEntry* qParticle;
			TreeEntry* wParticle;
		}
		namespace hitchance {
			TreeEntry* qHitchance;
			TreeEntry* wHitchance;
		}
		TreeEntry* lowSpec;
		TreeEntry* debugPrint;
	}

	static constexpr float SERVER_TICKRATE = 1000.f / 30.f;
	static constexpr float BRAND_W_PARTICLE_TIME = 0.6f;
	static constexpr float BRAND_Q_RANGE = 1050;
	static constexpr float BRAND_W_RANGE = 900;
	static constexpr float BRAND_E_RANGE = 660;
	static constexpr float BRAND_E_MAX_BOUNCE_RANGE = 600;
	static constexpr float BRAND_E_MIN_BOUNCE_RANGE = 300;
	static constexpr float BRAND_R_RANGE = 750;
	static constexpr float BRAND_R_BOUNCE_RANGE = 600;
	static constexpr float BRAND_R_MIN_SPEED = 750;
	//static constexpr float BRAND_R_MAX_SPEED = 3000;

	vector nexusPos;
	vector urfCannon;

	buff_instance_script elderBuff;

	game_object_script spamETarget;

	TreeTab* aurora_prediction;

	bool hasCasted = false;
	bool isQReady = false;
	bool isWReady = false;
	bool isEReady = false;
	bool isRReady = false;

	float last_tick = 0;
	float attackOrderTime = 0;
	float lastCast = 0;

	void debugPrint(const std::string& str, ...)
	{
		// Thanks seidhr
		if (settings::debugPrint->get_bool())
		{
			va_list                     list;
			int                         size;
			std::unique_ptr< char[] >   buf;

			if (str.empty())
				return;

			va_start(list, str);

			// count needed size.
			size = std::vsnprintf(0, 0, str.c_str(), list) + 1;

			// allocate.
			buf = std::make_unique< char[] >(size);
			if (!buf) {
				va_end(list);
				return;
			}

			// print to buffer.
			std::vsnprintf(buf.get(), size, str.c_str(), list);

			va_end(list);

			// print to console.
			console->print("%.*s", size - 1, buf.get());
		}
	}

	bool isUnderTower(const game_object_script& target)
	{
		for (const auto& turret : entitylist->get_enemy_turrets())
			if (target->get_position().distance(turret->get_position()) <= 750 + target->get_bounding_radius())
				return true;
		return false;
	}

	bool hasEnoughMana(const spellslot& spellslot)
	{
		switch (spellslot)
		{
		case spellslot::q:
		{
			return myhero->get_mana() >= qMana[myhero->get_spell(spellslot)->level() - 1];
		}
		case spellslot::w:
			return myhero->get_mana() >= wMana[myhero->get_spell(spellslot)->level() - 1];
		case spellslot::e:
			return myhero->get_mana() >= eMana[myhero->get_spell(spellslot)->level() - 1];
		case spellslot::r:
			return myhero->get_mana() >= rMana[myhero->get_spell(spellslot)->level() - 1];
		default:
			return true;
		}
	}

	bool can_cast(const spellslot& spellslot)
	{
		const auto spell = myhero->get_spell(spellslot);
		if (!spell || !myhero->can_cast())
		{
			return false;
		}

		const auto state = myhero->get_spell_state(spellslot);
		if (state == spell_state::Ready)
		{
			return true;
		}

		if (!hasEnoughMana(spellslot))
		{
			return false;
		}

		const auto cooldown = spell->cooldown();
		//IN CD = (state & (1 << 5)) != 0
		return cooldown < (getPing()) + 0.033f && (state & (1 << 5)) != 0;
	}

	void drawCircle(vector pos, int radius, int quality, bool legsense, unsigned long color, int thickness = 1)
	{
		if (legsense)
		{
			draw_manager->add_circle_with_glow(pos, color, radius, thickness, glow_data(0.1f, 0.75f, 0.f, 1.f));
			return;
		}
		const auto points = geometry::geometry::circle_points(pos, radius, quality);
		for (int i = 0; i < points.size(); i++)
		{
			const int next_index = (i + 1) % points.size();
			const auto start = points[i];
			const auto end = points[next_index];

			vector screenPosStart;
			renderer->world_to_screen(start, screenPosStart);
			vector screenPosEnd;
			renderer->world_to_screen(end, screenPosEnd);
			if (!renderer->is_on_screen(screenPosStart, 50) && !renderer->is_on_screen(screenPosEnd, 50))
				continue;

			draw_manager->add_line(points[i].set_z(pos.z), points[next_index].set_z(pos.z), color, thickness);
		}
	}

	bool isMoving(const game_object_script& target)
	{
		return target->get_path_controller() && !target->get_path_controller()->is_dashing() && target->is_moving();
	}

	float timeBeforeWHits(const game_object_script& target)
	{
		// Get time to hit before any W particle hits target (including ally W particles, useful in one for all)
		float returnTimeToHit = FLT_MAX;
		if (!target) return returnTimeToHit;
		for (const auto& particle : particleList) {
			const auto& timeBeforeHit = particle.creationTime + BRAND_W_PARTICLE_TIME - gametime->get_time();
			const auto& unitPositionDist = prediction->get_prediction(target, std::max(0.f, timeBeforeHit)).get_unit_position().distance(particle.particle->get_position());
			if (particle.particle->is_valid() && unitPositionDist <= w->get_radius() && returnTimeToHit > timeBeforeHit)
				returnTimeToHit = timeBeforeHit;
		}
		return returnTimeToHit;
	}

	float timeBeforeWHitsLocation(vector& position)
	{
		// Get time to hit before any W particle hits a specific location (including ally W particles, useful in one for all)
		float returnTimeToHit = FLT_MAX;
		for (const auto& particle : particleList) {
			const auto& timeBeforeHit = particle.creationTime + BRAND_W_PARTICLE_TIME - gametime->get_time();
			const auto& unitPositionDist = position.distance(particle.particle->get_position());
			if (particle.particle->is_valid() && unitPositionDist <= w->get_radius() && returnTimeToHit > timeBeforeHit)
				returnTimeToHit = timeBeforeHit;
		}
		return returnTimeToHit;
	}

	float getPing()
	{
		// Get player's full ping (ping pong)
		return ping->get_ping() / 1000;
	}

	float getGodBuffTime(const game_object_script& target)
	{
		// This function gets the god buff time (godmode && spellshield buffs) on a target, it's no use to attack them while they are immortal
		float buffTime = 0;
		for (auto&& buff : target->get_bufflist())
		{
			if (buff == nullptr || !buff->is_valid() || !buff->is_alive()) continue;

			const auto& buffHash = buff->get_hash_name();
			if (std::find(std::begin(godBuffList), std::end(godBuffList), buffHash) != std::end(godBuffList))
			{
				const auto& isPantheonE = buffHash == buff_hash("PantheonE");
				const auto& realRemainingTime = !isPantheonE ? buff->get_remaining_time() : buff->get_remaining_time() + 0.2;
				if (buffTime < realRemainingTime && (!isPantheonE || target->is_facing(myhero)) && (buffHash != buff_hash("XinZhaoRRangedImmunity") || myhero->get_position().distance(target->get_position()) > 450))
				{
					buffTime = realRemainingTime;
				}
			}
		}
		return buffTime;
	}

	float getNoKillBuffTime(const game_object_script& target)
	{
		// This function gets the no kill buff time on a target, you don't want to try killing people with these buffs
		float buffTime = 0;
		for (auto&& buff : target->get_bufflist())
		{
			if (buff == nullptr || !buff->is_valid() || !buff->is_alive()) continue;
			const auto& buffHash = buff->get_hash_name();
			if (std::find(std::begin(noKillBuffList), std::end(noKillBuffList), buffHash) != std::end(noKillBuffList))
			{
				if (buffTime < buff->get_remaining_time())
				{
					buffTime = buff->get_remaining_time();
				}
			}
		}
		return buffTime;
	}

	float getStasisTime(const game_object_script& target)
	{
		// This function gets the stasis time of a target
		float buffTime = 0;
		for (auto&& buff : target->get_bufflist())
		{
			if (buff == nullptr || !buff->is_valid() || !buff->is_alive()) continue;
			const auto& buffHash = buff->get_hash_name();
			if (std::find(std::begin(stasisBuffList), std::end(stasisBuffList), buffHash) != std::end(stasisBuffList))
			{
				if (buffTime < buff->get_remaining_time())
				{
					buffTime = buff->get_remaining_time();
				}
			}
		}
		// Get guardian angel revive time if there is one
		float GATime = (buffTime <= 0 && guardianReviveTime[target->get_handle()] ? guardianReviveTime[target->get_handle()] - gametime->get_time() : 0);
		if (buffTime < GATime)
		{
			buffTime = GATime;
		}
		return buffTime;
	}

	buffList combinedBuffChecks(const game_object_script& target)
	{
		// This function gets every single buffs that are needed making the 3 functions above completely useless!
		float godBuffTime = 0;
		float noKillBuffTime = 0;
		float stasisTime = 0;
		float stasisStart = 0;
		float stasisEnd = 0;
		for (auto&& buff : target->get_bufflist())
		{
			if (buff == nullptr || !buff->is_valid() || !buff->is_alive()) continue;

			const auto& buffHash = buff->get_hash_name();
			if (std::find(std::begin(godBuffList), std::end(godBuffList), buffHash) != std::end(godBuffList))
			{
				const auto& isPantheonE = buffHash == buff_hash("PantheonE");
				const auto& realRemainingTime = !isPantheonE ? buff->get_remaining_time() : buff->get_remaining_time() + 0.2;
				if (godBuffTime < realRemainingTime && (!isPantheonE || target->is_facing(myhero)) && (buffHash != buff_hash("XinZhaoRRangedImmunity") || myhero->get_position().distance(target->get_position()) > 450))
				{
					godBuffTime = realRemainingTime;
				}
			}
			else if (std::find(std::begin(noKillBuffList), std::end(noKillBuffList), buffHash) != std::end(noKillBuffList))
			{
				if (noKillBuffTime < buff->get_remaining_time())
				{
					noKillBuffTime = buff->get_remaining_time();
				}
			}
			else if (std::find(std::begin(stasisBuffList), std::end(stasisBuffList), buffHash) != std::end(stasisBuffList))
			{
				if (stasisTime < buff->get_remaining_time())
				{
					stasisTime = buff->get_remaining_time();
					stasisStart = buff->get_start();
					stasisEnd = buff->get_end();
				}
			}
		}
		// Get guardian angel revive time if there is one
		float GATime = (stasisTime <= 0 && guardianReviveTime[target->get_handle()] ? guardianReviveTime[target->get_handle()] - gametime->get_time() : 0);
		if (stasisTime < GATime)
		{
			stasisTime = GATime;
			stasisStart = guardianReviveTime[target->get_handle()] - 4;
			stasisEnd = guardianReviveTime[target->get_handle()];
		}
		const stasisStruct& stasisInfo = { .stasisTime = stasisTime, .stasisStart = stasisStart, .stasisEnd = stasisEnd };
		const buffList& buffStruct = { .godBuff = godBuffTime, .noKillBuff = noKillBuffTime, .stasis = stasisInfo };
		return buffStruct;
	}

	float getTotalHP(const game_object_script& target)
	{
		// Get total magic damage HP
		return target->get_health() + target->get_all_shield() + target->get_magical_shield();
	}

	bool couldDamageLater(const game_object_script& target, const float time, float damage = -1)
	{
		// Check if the time to hit target is bigger than their godmode buff remaining time or if we'll kill a target that we shouldn't try to kill
		if (!target->is_ai_hero()) return true;
		const auto& totalTime = std::max(0.f, time + getPing());
		if (damage >= 0 && damage < 100) damage = 100;
		if (godBuffTime[target->get_handle()] <= totalTime && (noKillBuffTime[target->get_handle()] <= totalTime || damage < getTotalHP(target)))
			return true;
		return false;
	}

	float getTimeToHit(prediction_input& input, prediction_output& predInfo, const bool takePing)
	{
		// Get time before spell hits
		if (predInfo.get_cast_position() == vector::zero) return FLT_MAX;
		const auto& timeToHit = (input._from.distance(predInfo.get_cast_position()) / input.speed) + input.delay + (takePing ? getPing() : 0);
		return timeToHit;
	}

	int alliesAroundTarget(const game_object_script& target, float range = FLT_MAX)
	{
		// Get amount of allies near target
		return myhero->get_position().distance(target->get_position()) >= range ? target->count_enemies_in_range(range) : target->count_enemies_in_range(range) - 1;
	}

	hit_chance getPredIntFromSettings(int hitchance)
	{
		// Get hitchance from settings value
		return static_cast<hit_chance>(hitchance + 3);
	}

	bool castQ(const game_object_script& target, std::string mode, const bool eCombo, bool ignoreHitChance = false)
	{
		// Cast Q
		if (hasCasted) return true;

		auto& p = qPredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = getTimeToHit(p.input, p, true);
		const auto& trueTimeToHit = getTimeToHit(p.input, p, false);
		auto pos = target->get_position();
	 	const auto& wTime = !ignoreHitChance ? timeBeforeWHits(target) : timeBeforeWHitsLocation(pos) - 0.2;
	 	const auto& ablazeBuff = target->get_buff(buff_hash("BrandAblaze"));
		const auto& targetAblaze = (ablazeBuff && ablazeBuff->get_remaining_time() >= timeToHit + 0.2);
		const auto& performECombo = settings::automatic::qeLogic->get_bool() && (eCombo && couldDamageLater(target, e->get_delay() - 0.1, eDamageList[target->get_handle()]) && trueTimeToHit > 0.5 && target->get_position().distance(myhero->get_position()) <= BRAND_E_RANGE && prediction->get_prediction(target, 0.25).get_unit_position().distance(myhero->get_position()) <= BRAND_E_RANGE && !targetAblaze);
	 	const auto& isQStun = targetAblaze || (wTime < timeToHit - 0.2) || performECombo;
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
	 	if ((ignoreHitChance || p.hitchance >= getPredIntFromSettings(settings::hitchance::qHitchance->get_int()) || !target->is_visible()) && aliveWhenLanding && (isQStun || qDamageList[target->get_handle()] > getTotalHP(target)) && couldDamageLater(target, trueTimeToHit - 0.2, qDamageList[target->get_handle()]))
	 	{
			lastCast = gametime->get_time() + 0.133 + getPing();
	 		q->cast(p.get_cast_position());
			if (performECombo)
				spamETarget = target;
	 		hasCasted = true;
			debugPrint("[%i:%02d] Casted Q on hitchance %i on target %s", (int)gametime->get_time() / 60, (int)gametime->get_time() % 60, p.hitchance, target->get_model_cstr());
			return true;
	 	}
		return false;
	}

	bool castW(const game_object_script& target, std::string mode, bool ignoreHitChance = false)
	{
		// Cast W
		if (hasCasted) return true;

		auto& p = wPredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = w->get_delay() + getPing();
		const auto& trueTimeToHit = w->get_delay();
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if ((ignoreHitChance || p.hitchance >= getPredIntFromSettings(settings::hitchance::wHitchance->get_int()) || !target->is_visible()) && aliveWhenLanding && couldDamageLater(target, trueTimeToHit - 0.2, wDamageList[target->get_handle()]))
		{
			lastCast = gametime->get_time() + 0.133 + getPing();
			w->cast(p.get_cast_position());
			hasCasted = true;
			debugPrint("[%i:%02d] Casted W on hitchance %i on target %s", (int)gametime->get_time() / 60, (int)gametime->get_time() % 60, p.hitchance, target->get_model_cstr());
			return true;
		}
		return false;
	}

	bool castE(const game_object_script& target, std::string mode)
	{
		// Cast E
		if (hasCasted) return true;
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, e->get_delay() + 0.2, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (couldDamageLater(target, e->get_delay() - 0.1, eDamageList[target->get_handle()]) && aliveWhenLanding && target->is_visible())
		{
			lastCast = gametime->get_time() + 0.133 + getPing();
			e->cast(target);
			return true;
		}
		return false;
	}

	bool castR(const game_object_script& target, std::string mode)
	{
		// Cast R
		if (hasCasted) return true;

		const auto& timeToHit = (myhero->get_position().distance(target->get_position()) / BRAND_R_MIN_SPEED) + r->get_delay() + getPing();
		const auto& trueTimeToHit = (myhero->get_position().distance(target->get_position()) / BRAND_R_MIN_SPEED) + r->get_delay() + getPing();
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit - 0.2, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (couldDamageLater(target, trueTimeToHit - 0.1, rDamageList[target->get_handle()].damage) && aliveWhenLanding && target->is_visible())
		{
			lastCast = gametime->get_time() + 0.133 + getPing();
			r->cast(target);
			return true;
		}
		return false;
	}

	prediction_output getQPred(const game_object_script& target)
	{
		// Get Q pred
		const auto& totalRadius = std::max(target->get_bounding_radius(), 65.f);
		q->set_range(BRAND_Q_RANGE + std::max(target->get_bounding_radius(), 65.f));
		q->from = myhero->get_position().distance(target->get_position()) > totalRadius ? myhero->get_position().extend(target->get_position(), totalRadius) : target->get_position();
		prediction_output p = q->get_prediction(target);
		if (p.hitchance <= static_cast<hit_chance>(2)) return p;

		//Behind yourself collision detection
		const auto& collisionsFromHero = q->get_collision(myhero->get_position().extend(target->get_position(), -q->get_radius()), { p.input.get_from() });
		if (!collisionsFromHero.empty()) return prediction_output{};

		return p;
	}

	prediction_output getWPred(const game_object_script& target)
	{
		// Get W pred

		// Delay is randomly higher
		//w->set_delay(isMoving(target) ? 0.95 : 0.9);

		const prediction_output& p = w->get_prediction(target);
		return p;
	}

	bool qCanBeCasted(const game_object_script& target)
	{
		// Check if could cast Q in a near future
		auto& p = qPredictionList[target->get_handle()];
		const auto& timeToHit = getTimeToHit(p.input, p, true);
		const auto& trueTimeToHit = getTimeToHit(p.input, p, false);
		const auto& wTime = timeBeforeWHits(target);
		const auto& ablazeBuff = target->get_buff(buff_hash("BrandAblaze"));
		const auto& targetAblaze = (ablazeBuff && ablazeBuff->get_remaining_time() >= timeToHit);
		const auto& isQStun = targetAblaze || (wTime < timeToHit + 0.15);
		const auto& qIsReady = isQReady && couldDamageLater(target, trueTimeToHit - 0.2, qDamageList[target->get_handle()]) && qPredictionList[target->get_handle()].hitchance > getPredIntFromSettings(settings::hitchance::qHitchance->get_int());
		return isQStun && qIsReady;
	}

	float getExtraDamage(const game_object_script& target, const int shots, const float predictedHealth, const float damageDealt, const bool isCC, const bool firstShot, const bool isTargeted, const int passiveStacks)
	{
		// Get extra damage based off items && runes && drake souls
		float damage = 0;
		const auto& bonusAD = myhero->get_total_attack_damage() - myhero->get_base_attack_damage();
		const auto& level = myhero->get_level();
		const auto& abilityPower = myhero->get_total_ability_power();
		const auto& buff3 = myhero->get_buff(buff_hash("ASSETS/Perks/Styles/Inspiration/FirstStrike/FirstStrikeAvailable.lua"));
		const auto& buff4 = myhero->get_buff(buff_hash("ASSETS/Perks/Styles/Inspiration/FirstStrike/FirstStrike.lua"));
		const auto& targetMaxHealth = target->get_max_health();
		if (shots <= 0)
		{
			const auto& buff1 = myhero->get_buff(buff_hash("ASSETS/Perks/Styles/Domination/DarkHarvest/DarkHarvest.lua"));
			const auto& buff2 = myhero->get_buff(buff_hash("ASSETS/Perks/Styles/Domination/DarkHarvest/DarkHarvestCooldown.lua"));
			const auto& buff5 = myhero->get_buff(buff_hash("SRX_DragonSoulBuffInfernal_Cooldown"));
			const auto& buff6 = myhero->get_buff(buff_hash("SRX_DragonSoulBuffInfernal"));
			const auto& buff7 = myhero->get_buff(buff_hash("SRX_DragonSoulBuffHextech"));
			const auto& buff8 = myhero->get_buff(buff_hash("srx_dragonsoulbuffhextech_cd"));
			const auto& buff9 = elderBuff;
			const auto& buff10 = target->get_buff(buff_hash("BrandAblaze"));
			if (buff1 && !buff2 && predictedHealth / targetMaxHealth < 0.5) {
				const auto& harvestDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, 20 + 40 / 17 * (level - 1) + abilityPower * 0.15 + bonusAD * 0.25 + buff1->get_count() * 5);
				damage = damage + harvestDamage;
			}
			if (buff6 && !buff5) {
				auto infernalDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, 80 + bonusAD * 0.225 + abilityPower * 0.15 + myhero->get_max_health() * 0.0275);
				damage = damage + infernalDamage;
			}
			if (buff7 && !buff8)
			{
				damage = damage + 25 + 25 / 17 * (level - 1);
			}
			if (buff9)
			{
				const auto& amountOfMins = std::floor(gametime->get_time() / 60);
				const auto& extraDamage = ((amountOfMins < 27) ? 75 : (amountOfMins < 45 ? (75 + ((amountOfMins - 25) / 2) * 15) : 225));
				damage = damage + extraDamage;
			}
			auto passiveCount = buff10 ? buff10->get_count() : 0;
			passiveCount = std::min(3, passiveCount + passiveStacks);
			const auto& passiveDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, (targetMaxHealth * 0.025) * passiveCount);
			damage = damage + passiveDamage;
			if ((!buff10 || (buff10 && buff10->get_count() < 3)) && passiveCount == 3)
			{
				const auto& damagePercent = (std::min(0.13, 0.0875 + 0.0025 * level) + 0.025);
				damage = damage + damagePercent;
			}
		}
		bool hasRylai = false;
		bool hasHorizon = false;
		for (int i = 11; i >= 6; i--)
		{
			auto item = myhero->get_item((spellslot)i);
			if (!item) continue;

			switch (item->get_item_id())
			{
			case (int)ItemId::Ludens_Tempest:
			{
				if (firstShot && myhero->get_spell((spellslot)i)->cooldown() <= 0)
				{
					const auto& ludensDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, 100 + abilityPower * 0.1);
					damage = damage + ludensDamage;
				}
				break;
			}
			case (int)ItemId::Hextech_Alternator:
			{
				if (firstShot && myhero->get_spell((spellslot)i)->cooldown() <= 0)
				{
					const auto& alternatorDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, 50 + 75 / 17 * (level - 1));
					damage = damage + alternatorDamage;
				}
				break;
			}
			case (int)ItemId::Liandrys_Anguish:
			{
				if (shots <= 0)
				{
					const auto& liandrysDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, 50 + (abilityPower * 0.06) + (targetMaxHealth * 0.04));
					damage = damage + liandrysDamage;
				}
				break;
			}
			case (int)ItemId::Demonic_Embrace:
			{
				if (shots <= 0)
				{
					const auto& demonicDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, targetMaxHealth * 0.04);
					damage = damage + demonicDamage;
				}
				break;
			}
			case (int)ItemId::Horizon_Focus:
			{
				hasHorizon = true;
				break;
			}
			case (int)ItemId::Rylais_Crystal_Scepter:
			{
				hasRylai = true;
				break;
			}
			}
		}
		if (hasHorizon && (isCC || hasRylai || (!isTargeted && myhero->get_position().distance(target->get_position()) > 700) || target->get_buff(buff_hash("4628marker"))))
		{
			damage = damage + (damageDealt) * 0.1;
		}
		if (buff3 || buff4)
		{
			damage = damage + (damage + damageDealt) * 0.09;
		}
		return damage;
	}

	float getQDamage(const game_object_script& target)
	{
		// Get Q damage
		const auto& spell = myhero->get_spell(spellslot::q);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 50 + spell->level() * 30 + myhero->get_total_ability_power() * 0.55;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, false, true, false, 1);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getWDamage(const game_object_script& target)
	{
		// Get W normal damage
		const auto& spell = myhero->get_spell(spellslot::w);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 30 + 45 * spell->level() + myhero->get_total_ability_power() * 0.60;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, true, true, false, 1);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getW2Damage(const game_object_script& target)
	{
		// Get W empowered damage
		const auto& spell = myhero->get_spell(spellslot::w);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = (30 + 45 * spell->level() + myhero->get_total_ability_power() * 0.60) * 1.25;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, true, true, false, 1);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getEDamage(const game_object_script& target)
	{
		// Get E damage
		const auto& spell = myhero->get_spell(spellslot::e);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 45 + 25 * spell->level() + myhero->get_total_ability_power() * 0.45;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, true, true, true, 1);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getRDamage(const game_object_script& target, const int shots, const float predictedHealth, const bool firstShot, const int passiveStacks)
	{
		// Get R damage
		const auto& spell = myhero->get_spell(spellslot::r);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 100 * spell->level() + myhero->get_total_ability_power() * 0.25;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, shots, predictedHealth, damageLibDamage, false, firstShot, true, passiveStacks);
		if (elderBuff && (predictedHealth - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = getTotalHP(target);
		return totalDamage;
	}

	rDamageData getTotalRDamage(const game_object_script& target)
	{
		// Get total damage of R && returns damage, bounces needed to kill && if it kills
		auto rDamage = 0.f;
		auto shotsToKill = 0;
		auto isFirstShot = true;
		const auto& totalHP = getTotalHP(target);
		const auto& rActive = myhero->get_spell(spellslot::r)->level() != 0 && myhero->get_spell(spellslot::r)->cooldown() <= 0;
		if (rActive)
		{
			for (int i = 3; i > 0; i--)
			{
				auto calculatedRDamage = getRDamage(target, i, totalHP - rDamage, isFirstShot, shotsToKill + 1);
				auto calculatedRMaxDamage = getRDamage(target, 0, totalHP - rDamage, isFirstShot, shotsToKill + 1);
				if (((totalHP)-(rDamage + calculatedRMaxDamage)) / target->get_max_health() < 0)
				{
					rDamage = rDamage + calculatedRMaxDamage;
					shotsToKill = shotsToKill + 1;
					break;
				}
				rDamage = rDamage + calculatedRDamage;
				shotsToKill = shotsToKill + 1;
				isFirstShot = false;
			}
			auto canBeKilled = rDamage >= totalHP;
			rDamageData rDataStruct = { .damage = rDamage, .shots = shotsToKill, .kills = canBeKilled };
			return rDataStruct;
		}
		rDamageData rDataStruct = { .damage = 0, .shots = 0, .kills = false };
		return rDataStruct;
	}

	void draw_dmg_rl(const game_object_script& target, const float damage, unsigned long color)
	{
		// Draw damage on HP bar from right to left
		if (target != nullptr && target->is_valid() && target->is_hpbar_recently_rendered())
		{
			auto bar_pos = target->get_hpbar_pos();

			if (bar_pos.is_valid() && !target->is_dead() && target->is_visible())
			{
				const auto& health = target->get_real_health();

				bar_pos = vector(bar_pos.x + (105 * (health / target->get_max_health())), bar_pos.y -= 10);

				auto damage_size = (105 * (damage / target->get_max_health()));

				if (damage >= health)
				{
					damage_size = (105 * (health / target->get_max_health()));
				}

				if (damage_size > 105)
				{
					damage_size = 105;
				}

				const auto& size = vector(bar_pos.x + (damage_size * -1), bar_pos.y + 11);

				draw_manager->add_filled_rect(bar_pos, size, color);
			}
		}
	}

	void draw_dmg_lr(const game_object_script& target, const float damage, unsigned long color)
	{
		// Draw damage on HP bar from left to right
		if (target != nullptr && target->is_valid() && target->is_hpbar_recently_rendered())
		{
			auto bar_pos = target->get_hpbar_pos();

			if (bar_pos.is_valid() && !target->is_dead() && target->is_visible())
			{
				const auto& health = target->get_real_health();

				bar_pos = vector(bar_pos.x, bar_pos.y -= 10);

				auto damage_size = (105 * (damage / target->get_max_health()));

				if (damage >= health)
				{
					damage_size = (105 * (health / target->get_max_health()));
				}

				if (damage_size > 105)
				{
					damage_size = 105;
				}

				const auto& size = vector(bar_pos.x + damage_size, bar_pos.y + 11);

				draw_manager->add_filled_rect(bar_pos, size, color);
			}
		}
	}

	bool isYuumiAttached(const game_object_script& target)
	{
		// Check if the target is Yuumi and if it's attached to someone
		if (target->get_champion() == champion_id::Yuumi)
		{
			const auto& yuumiBuff = target->get_buff(buff_hash("YuumiWAttach"));
			if (yuumiBuff && yuumiBuff->get_caster()->get_handle() == target->get_handle()) return true;
		}
		return false;
	}

	int isCastMoving(const game_object_script& target)
	{
		if (target->get_spell(spellslot::w)->get_name_hash() == spell_hash("NunuW_Recast") && target->is_playing_animation(buff_hash("Spell2")))
			return 2;
		if (target->get_spell(spellslot::w)->get_name_hash() == spell_hash("AurelionSolWToggle") && target->is_playing_animation(buff_hash("Spell2")))
			return 2;
		if (target->get_active_spell() && target->get_active_spell()->get_spell_data()->get_name_hash() == spell_hash("SionR"))
			return 1;
		return 0;
	}

	bool isRecalling(const game_object_script& target)
	{
		// Get if target is recalling
		const auto& isRecalling = target->is_teleporting() && (target->is_teleporting() || target->get_teleport_state() == "recall" || target->get_teleport_state() == "SuperRecall" || target->get_teleport_state() == "SummonerTeleport");
		return isRecalling;
	}

	bool isValidRecalling(const game_object_script& target, float range = FLT_MAX, const vector& from = vector::zero)
	{
		// Get valid recalling enemies in FoW
		auto fromPos = from;
		if (fromPos == vector::zero) fromPos = myhero->get_position();
		const auto& isValid = (target->is_valid() && target->is_ai_hero() && target->is_targetable() && target->is_targetable_to_team(myhero->get_team()) && !target->is_invulnerable() && isRecalling(target) && !target->is_dead() && fromPos.distance(target->get_position()) <= range);
		return isValid;
	}

	bool customIsValid(const game_object_script& target, float range = FLT_MAX, const vector& from = vector::zero, bool invul = false)
	{
		// Custom isValid

		// If it's Yuumi that is attached then target is not valid
		if (isYuumiAttached(target)) return false;

		if (aurora_prediction && aurora_prediction->is_hidden() == false && settings::automatic::fowPred->get_bool() && prediction->get_prediction(target, 0.F).hitchance > hit_chance::impossible)
			return true;

		const auto& isCastingImmortalitySpell = (target->get_active_spell() && std::find(std::begin(immuneSpells), std::end(immuneSpells), target->get_active_spell()->get_spell_data()->get_name_hash()) != std::end(immuneSpells)) || target->has_buff(buff_hash("AkshanE2"));
		const auto& isValid = !isCastingImmortalitySpell && ((target->is_valid_target(range, from, invul) && target->is_targetable() && target->is_targetable_to_team(myhero->get_team()) && !target->is_invulnerable()));
		return isValid;
	}

	bool rCollision(const game_object_script& target)
	{
		// Check if R collides with windwall from hero to target
		const auto& collisionsFromHero = r->get_collision(myhero->get_position(), { target->get_position() });
		const auto& blockedR = !collisionsFromHero.empty();
		return blockedR;
	}

	bool canRBounce(const game_object_script& target)
	{
		// Check if R can bounce on something including yourself
		if (myhero->get_position().distance(target->get_position()) <= BRAND_R_BOUNCE_RANGE) return true;
		std::vector<game_object_script> minionList;
		minionList.reserve(entitylist->get_enemy_minions().size() + entitylist->get_jugnle_mobs_minions().size() + entitylist->get_enemy_heroes().size() + entitylist->get_other_minion_objects().size());
		minionList.insert(minionList.end(), entitylist->get_enemy_minions().begin(), entitylist->get_enemy_minions().end());
		minionList.insert(minionList.end(), entitylist->get_jugnle_mobs_minions().begin(), entitylist->get_jugnle_mobs_minions().end());
		minionList.insert(minionList.end(), entitylist->get_enemy_heroes().begin(), entitylist->get_enemy_heroes().end());
		minionList.insert(minionList.end(), entitylist->get_other_minion_objects().begin(), entitylist->get_other_minion_objects().end());
		minionList.erase(std::remove_if(minionList.begin(), minionList.end(), [](game_object_script x)
			{
				return !customIsValid(x, BRAND_E_RANGE) || x->is_ally();
			}), minionList.end());
		for (const auto& minion : minionList)
		{
			if (minion->get_handle() == target->get_handle()) continue;
			auto collisions = r->get_collision(minion->get_position(), { target->get_position() });
			auto isColliding = !collisions.empty();
			if (isColliding) return false;

			const auto& timeToReachTarget = r->get_delay() + (myhero->get_position().distance(minion->get_position()) + target->get_position().distance(minion->get_position())) / BRAND_R_MIN_SPEED;
			const auto& rDelay = r->get_delay() + timeToReachTarget + getPing();
			const auto& distance = minion->get_position().distance(target->get_position());
			const auto& predictedDistance = prediction->get_prediction(minion, rDelay).get_unit_position().distance(prediction->get_prediction(target, rDelay).get_unit_position());
			if (distance <= BRAND_R_BOUNCE_RANGE && predictedDistance <= BRAND_R_BOUNCE_RANGE)
				return true;
		}
		return false;
	}

	int rBounceCount(const game_object_script& target)
	{
		// Get amount of targets that R can hit if it bounces on given target
		int count = 0;
		for (const auto& enemy : entitylist->get_enemy_heroes())
		{
			if (!customIsValid(enemy)) continue;

			if (enemy->get_handle() == target->get_handle() || godBuffTime[enemy->get_handle()] > 0) {
				count++;
				continue;
			}
			auto collisions = r->get_collision(target->get_position(), { enemy->get_position() });
			auto isColliding = !collisions.empty();
			if (isColliding) return 1;

			const auto& timeToReachTarget = r->get_delay() + (myhero->get_position().distance(enemy->get_position()) + target->get_position().distance(enemy->get_position())) / BRAND_R_MIN_SPEED;
			const auto& rDelay = r->get_delay() + timeToReachTarget + getPing();
			const auto& distance = enemy->get_position().distance(target->get_position());
			const auto& predictedDistance = prediction->get_prediction(enemy, rDelay).get_unit_position().distance(prediction->get_prediction(target, rDelay).get_unit_position());
			if (distance <= BRAND_R_BOUNCE_RANGE && predictedDistance <= BRAND_R_BOUNCE_RANGE)
				count++;
		}
		return count;
	}

	game_object_script findClosestMinion(const game_object_script& target)
	{
		// Find closest entity that R can bounce on (prio to champions)
		game_object_script prioTarget = nullptr;
		float distanceFromTarget = FLT_MAX;
		bool isChampion = false;
		std::vector<game_object_script> minionList;
		minionList.reserve(entitylist->get_enemy_minions().size() + entitylist->get_jugnle_mobs_minions().size() + entitylist->get_enemy_heroes().size() + entitylist->get_other_minion_objects().size());
		minionList.insert(minionList.end(), entitylist->get_enemy_minions().begin(), entitylist->get_enemy_minions().end());
		minionList.insert(minionList.end(), entitylist->get_jugnle_mobs_minions().begin(), entitylist->get_jugnle_mobs_minions().end());
		minionList.insert(minionList.end(), entitylist->get_enemy_heroes().begin(), entitylist->get_enemy_heroes().end());
		minionList.insert(minionList.end(), entitylist->get_other_minion_objects().begin(), entitylist->get_other_minion_objects().end());
		minionList.erase(std::remove_if(minionList.begin(), minionList.end(), [](game_object_script x)
			{
				return !customIsValid(x, BRAND_R_RANGE) || x->is_ally();
			}), minionList.end());
		for (const auto& minion : minionList)
		{
			if (!customIsValid(target, BRAND_R_BOUNCE_RANGE, minion->get_position()) || target->get_handle() == minion->get_handle()) continue;
			const auto& collisionsFromHero = r->get_collision(myhero->get_position(), { minion->get_position() });
			const auto& collisions = r->get_collision(minion->get_position(), { target->get_position() });
			const auto& isColliding = !collisions.empty() || !collisionsFromHero.empty();
			if (isColliding) continue;

			const auto& timeToReachTarget = r->get_delay() + (myhero->get_position().distance(minion->get_position()) + target->get_position().distance(minion->get_position())) / BRAND_R_MIN_SPEED;
			const auto& rDelay = r->get_delay() + timeToReachTarget + getPing();
			const auto& totalRange = BRAND_R_BOUNCE_RANGE;
			const auto& distance = minion->get_position().distance(target->get_position());
			const auto& isMinionChamp = minion->is_ai_hero() && godBuffTime[minion->get_handle()] <= 0;
			const auto& predictedDistance = prediction->get_prediction(minion, rDelay).get_unit_position().distance(prediction->get_prediction(target, rDelay).get_unit_position());
			if (distance <= totalRange && predictedDistance <= totalRange && (!isChampion || isMinionChamp) && (prioTarget == nullptr || distance < distanceFromTarget))
			{
				prioTarget = minion;
				distanceFromTarget = distance;
				isChampion = isMinionChamp;
			}
		}
		return prioTarget;
	}

	std::vector<eBounceTarget> getEBounceTargets()
	{
		// This function gets every possible targets to use E on to hit any enemy champion
		std::vector<eBounceTarget> bounceTargets;
		std::vector<game_object_script> minionList;
		minionList.reserve(entitylist->get_enemy_minions().size() + entitylist->get_jugnle_mobs_minions().size() + entitylist->get_enemy_heroes().size() + entitylist->get_other_minion_objects().size());
		minionList.insert(minionList.end(), entitylist->get_enemy_minions().begin(), entitylist->get_enemy_minions().end());
		minionList.insert(minionList.end(), entitylist->get_jugnle_mobs_minions().begin(), entitylist->get_jugnle_mobs_minions().end());
		minionList.insert(minionList.end(), entitylist->get_enemy_heroes().begin(), entitylist->get_enemy_heroes().end());
		minionList.insert(minionList.end(), entitylist->get_other_minion_objects().begin(), entitylist->get_other_minion_objects().end());
		minionList.erase(std::remove_if(minionList.begin(), minionList.end(), [](game_object_script x)
			{
				return !customIsValid(x, BRAND_E_RANGE) || x->is_ally();
			}), minionList.end());
		for (const auto& minion : minionList)
		{
			for (const auto& target : entitylist->get_enemy_heroes())
			{
				if (!customIsValid(target, BRAND_E_MAX_BOUNCE_RANGE, minion->get_position()) || target->get_handle() == minion->get_handle()) continue;
				const auto& collisions = e->get_collision(minion->get_position(), { target->get_position() });
				const auto& isColliding = !collisions.empty();
				if (isColliding) continue;

				const auto& ablazeBuff = minion->get_buff(buff_hash("BrandAblaze"));
				const auto& eDelay = e->get_delay() + getPing();
				const auto& totalRange = (ablazeBuff && ablazeBuff->get_remaining_time() >= eDelay) ? BRAND_E_MAX_BOUNCE_RANGE : BRAND_E_MIN_BOUNCE_RANGE;
				const auto& distance = minion->get_position().distance(target->get_position());
				const auto& predictedDistance = prediction->get_prediction(minion, eDelay).get_unit_position().distance(prediction->get_prediction(target, eDelay).get_unit_position());
				if (distance <= totalRange && predictedDistance <= totalRange)
				{
					bounceTargets.push_back({ .target = minion, .extraRange = totalRange==BRAND_E_MAX_BOUNCE_RANGE, .priority = priorityList[target->get_handle()], .distance = predictedDistance - (ablazeBuff ? 300 : 0)});
				}
			}
		}
		return bounceTargets;
	}

	game_object_script getBestETarget(std::vector<eBounceTarget> bounceTargetList)
	{
		// This function gets best E target out of every targets given
		game_object_script prioTarget = nullptr;
		float distance = FLT_MAX;
		float priority = 0;
		for (const auto& target : eBounceTargets)
		{
			if (!customIsValid(target.target, BRAND_E_RANGE)) continue;

			if (prioTarget == nullptr || target.priority > priority || (target.priority == priority && target.distance < distance))
			{
				prioTarget = target.target;
				distance = target.distance;
				priority = target.priority;
			}
		}
		return prioTarget;
	}

	bool limitedTick(float msTime)
	{
		// Only execute once per msTime
		if (gametime->get_time() - last_tick <= msTime / 1000) return true;

		return false;
	}

	void calcs()
	{
		// Register last time update triggered (for low spec mode)
		last_tick = gametime->get_time();

		// Manage auto attacks
		orbwalker->set_attack((attackOrderTime > gametime->get_time() - 0.066) ? ((orbwalker->combo_mode()) ? false : true) : true);

		// Get aurora pred menu
		aurora_prediction = menu->get_tab("aurora_prediction");

		// Allows casting a spell for this update
		hasCasted = false;

		// Get ready spells
		isQReady = can_cast(spellslot::q);
		isWReady = can_cast(spellslot::w);
		isEReady = can_cast(spellslot::e);
		isRReady = can_cast(spellslot::r);

		// Get buffs
		elderBuff = myhero->get_buff(buff_hash("ElderDragonBuff"));

		// Get pred & damage of spells && a bunch of useful stuff on every enemies so you don't need to do it multiple times per update
		for (const auto& target : entitylist->get_enemy_heroes())
		{
			if (!target->is_valid() || target->is_dead()) continue;

			qPredictionList[target->get_handle()] = isQReady ? getQPred(target) : prediction_output {};
			wPredictionList[target->get_handle()] = isWReady ? getWPred(target) : prediction_output {};

			stunTime[target->get_handle()] = target->get_immovibility_time();
			qDamageList[target->get_handle()] = getQDamage(target);
			wDamageList[target->get_handle()] = getW2Damage(target);
			eDamageList[target->get_handle()] = getEDamage(target);
			rDamageList[target->get_handle()] = getTotalRDamage(target);

			// Remove guardian angel time if target finished revive
			if (!target->is_playing_animation(buff_hash("Death")))
				guardianReviveTime[target->get_handle()] = -1;

			if (!target->is_visible()) continue;

			// Get every important buff times
			buffList listOfNeededBuffs = combinedBuffChecks(target);
			godBuffTime[target->get_handle()] = listOfNeededBuffs.godBuff;
			noKillBuffTime[target->get_handle()] = listOfNeededBuffs.noKillBuff;
			stasisInfo[target->get_handle()] = listOfNeededBuffs.stasis;
		}

		// Get every E bounce targets
		eBounceTargets = getEBounceTargets();

		// Get best E target
		bestETarget = getBestETarget(eBounceTargets);

	}

	bool eSpam() {
		// Spams the fuck out of E for Q-E combo
		if (!spamETarget || !isEReady) return false;
		if (myhero->get_active_spell() && myhero->get_active_spell()->get_spellslot() == q->get_slot())
		{
			e->cast(spamETarget);
			return true;
		}
		else
		{
			spamETarget = nullptr;
		}
		return false;
	}

	bool debuffCantCast()
	{
		// Check if player can cast
		return !myhero->can_cast() || myhero->has_buff_type({ buff_type::Stun, buff_type::Asleep, buff_type::Fear, buff_type::Flee, buff_type::Charm, buff_type::Berserk, buff_type::Silence, buff_type::Taunt, buff_type::Suppression, buff_type::Knockback, buff_type::Knockup });
	}

	bool isCastingSpell()
	{
		// Check if we're already casting a spell
		const auto& castingTime = myhero->get_active_spell() ? myhero->get_active_spell()->cast_start_time() - gametime->get_time() : 0;
		if (myhero->get_active_spell() && !myhero->get_active_spell()->is_auto_attack())
			if (castingTime > getPing() + 0.033 && castingTime > 0)
				return !myhero->get_active_spell()->is_channeling() && !myhero->get_active_spell()->get_spell_data()->is_insta();
		return false;
	}

	bool isCastingAuto()
	{
		// Check if we're casting an auto
		const auto& castingTime = myhero->get_active_spell() ? myhero->get_active_spell()->cast_start_time() - gametime->get_time() : 0;
		if (myhero->get_active_spell() && myhero->get_active_spell()->is_auto_attack())
			if (castingTime > getPing() - 0.033 && castingTime > 0)
				return true;
		return false;
	}

	bool canCastSpells()
	{
		if (myhero->is_dead()) return false;

		if (myhero->is_recalling()) return false;

		if (lastCast > gametime->get_time()) return false;

		if (!isQReady && !isWReady && !isEReady && !isRReady) return false;

		if (debuffCantCast()) return false;

		if (isCastingSpell()) return false;

		if (settings::automatic::attackCheck->get_bool() && isCastingAuto()) return false;

		return true;
	}

	void targetSelectorSort()
	{
		// Sort targets based off TS prio
		targets = entitylist->get_enemy_heroes();
		targets.erase(std::remove_if(targets.begin(), targets.end(), [](const game_object_script& x)
			{
				return !x || !x->is_valid();
			}
		),
			targets.end());
		std::vector<game_object_script> dummyList;
		const auto size = targets.size();
		auto currentPrio = targets.size();
		for (int i = 0; i < size; i++)
		{
			const auto& tsTarget = target_selector->get_target(targets, damage_type::magical);
			if (tsTarget)
			{
				dummyList.push_back(tsTarget);
				priorityList[tsTarget->get_handle()] = currentPrio;
				currentPrio--;
				targets.erase(std::remove_if(targets.begin(), targets.end(), [dummyList](const game_object_script& x)
					{
						for (const auto& target : dummyList)
						{
							if (target && x && target->get_handle() == x->get_handle())
								return true;
						}
						return false;
					}
				),
					targets.end());
			}
		}
		std::sort(dummyList.begin(), dummyList.end(), [](game_object_script a, game_object_script b) {
			return target_selector->get_selected_target() && target_selector->get_selected_target()->get_handle() == a->get_handle();
			}
		);
		targets = dummyList;
	}

	void combo()
	{
		// Check if combo && if didn't cast yet
		if (!orbwalker->combo_mode() || hasCasted) return;

		// Loop through every sorted targets
		for (const auto& target : targets)
		{
			// Valid target check
			const bool& isValidTarget = target && customIsValid(target) && !target->is_zombie();

			// If not valid then go to next target
			if (!isValidTarget) continue;

			// Store useful info to use in logic
			const auto& ccTime = stunTime[target->get_handle()];
			auto& qp = qPredictionList[target->get_handle()];
			const auto& qLandingTime = getTimeToHit(qp.input, qp, true);
			const auto& trueQLandingTime = getTimeToHit(qp.input, qp, false);

			// Check if X spells can be used on that target
			const auto& dist = target->get_position().distance(myhero->get_position());
			const auto& canUseQ = settings::combo::qCombo->get_bool() && couldDamageLater(target, trueQLandingTime - 0.5, qDamageList[target->get_handle()]) && isQReady && qPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canUseW = settings::combo::wCombo->get_bool() && couldDamageLater(target, w->get_delay() - 0.5, wDamageList[target->get_handle()]) && isWReady && wPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canUseE = settings::combo::eCombo->get_bool() && couldDamageLater(target, e->get_delay() - 0.1, eDamageList[target->get_handle()]) && isEReady && (dist <= BRAND_E_RANGE || (settings::combo::eExtraCombo->get_bool() ? bestETarget : nullptr)) && target->is_visible();
			const auto& rRange = settings::combo::rComboKills->get_bool() && settings::combo::rComboMinionBounce->get_bool() ? 1350 : BRAND_R_RANGE;
			const auto& canUseR = settings::combo::rCombo->get_bool() && couldDamageLater(target, r->get_delay() - 0.5, rDamageList[target->get_handle()].damage) && isRReady && dist <= rRange && target->is_visible();
			const auto& couldUseQ = (canUseQ && qCanBeCasted(target));
			const auto& couldUseE = settings::combo::eCombo->get_bool() && isEReady;

			// If no spells can be used on that target then go to next target
			if (!canUseQ && !canUseW && !canUseE && !canUseR) continue;

			// Cancel autos if can E or if could Q
			if (couldUseE || couldUseQ) attackOrderTime = gametime->get_time();

			// Store data about R
			const auto& isRBlocked = rCollision(target);
			const auto& rCanBounce = !isRBlocked ? canRBounce(target) : false;
			const int& rAoECount = !isRBlocked ? rBounceCount(target): 0;
			const auto& rMinionBounce = dist > BRAND_R_RANGE;
			const auto& amountOfShots = rCanBounce ? std::min((rMinionBounce ? 2 : 3), settings::combo::rComboBounces->get_int()) : 1;
			const bool& rKills = settings::combo::rComboKills->get_bool() && rDamageList[target->get_handle()].kills && rDamageList[target->get_handle()].shots > 0 && rDamageList[target->get_handle()].shots <= amountOfShots;
			
			auto shouldBreak = false;

			// Q cast
			if (canUseQ && (ccTime - 0.3) < qLandingTime && castQ(target, "combo", canUseE)) return;

			// W cast
			if (canUseW)
			{
				shouldBreak = true;
				if (castW(target, "combo")) return;
			}

			// E cast
			if (canUseE && !couldUseQ)
			{
				if (target->get_position().distance(myhero->get_position()) <= BRAND_E_RANGE)
				{
					if (castE(target, "combo")) return;
				}
				else if (bestETarget)
				{
					if (castE(bestETarget, "combobounce")) return;
				}
			}

			// R cast
			if (canUseR)
			{
				if (dist <= BRAND_R_RANGE && !isRBlocked)
				{
					if (settings::combo::rComboAoE->get_bool() && rAoECount >= settings::combo::rComboAoEAmount->get_int())
					{
						if (castR(target, "comboaoe")) return;
					}
					const auto& comboLogic = !settings::combo::rComboLogic
						|| prediction->get_prediction(target, 0.5).get_unit_position().distance(myhero->get_position()) > BRAND_R_RANGE
						|| ((!canUseQ && !canUseW && !canUseE) && alliesAroundTarget(target, 500) < 1
							&& ((target->get_health() - health_prediction->get_incoming_damage(target, 3, true) > 100)
								|| (myhero->get_health() - health_prediction->get_incoming_damage(myhero, 3, true) < 150)));
					if (rKills && comboLogic)
					{
						if (castR(target, "combokill")) return;
					}
				}
				else if (rMinionBounce && rKills)
				{
					auto prioTarget = findClosestMinion(target);
					if (prioTarget)
						if (castR(prioTarget, "combobouncekill")) return;
				}
			}

			if (shouldBreak) break;

		}
	}

	void harass()
	{
		// Check if harass && if didn't cast yet
		if (!orbwalker->harass() || hasCasted) return;

		// Loop through every sorted targets
		for (const auto& target : targets)
		{
			// Valid target check
			bool isValidTarget = target && customIsValid(target) && !target->is_zombie();

			// If not valid then go to next target
			if (!isValidTarget) continue;

			// Check if X spells can be used on that target
			const auto& dist = target->get_position().distance(myhero->get_position());
			const auto& canUseQ = settings::harass::qHarass->get_bool() && couldDamageLater(target, q->get_delay() - 0.5, qDamageList[target->get_handle()]) && isQReady && qPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canUseW = settings::harass::wHarass->get_bool() && couldDamageLater(target, w->get_delay() - 0.5, wDamageList[target->get_handle()]) && isWReady && wPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canUseE = settings::harass::eHarass->get_bool() && couldDamageLater(target, e->get_delay() - 0.1, eDamageList[target->get_handle()]) && isEReady && (dist <= BRAND_E_RANGE || (settings::combo::eExtraCombo->get_bool() ? bestETarget : nullptr)) && target->is_visible();
			const auto& couldUseQ = (canUseQ && qCanBeCasted(target));

			// If no spells can be used on that target then go to next target
			if (!canUseQ && !canUseW && !canUseE) continue;

			// Store useful info to use in logic
			const auto& ccTime = stunTime[target->get_handle()];
			auto shouldBreak = false;

			// Q cast
			if (canUseQ && castQ(target, "harass", canUseE)) return;

			// W cast
			if (canUseW)
			{
				shouldBreak = true;
				if (castW(target, "harass")) return;
			}

			// E cast
			if (canUseE && !couldUseQ)
			{
				if (target->get_position().distance(myhero->get_position()) <= BRAND_E_RANGE)
				{
					if (castE(target, "harass")) return;
				}
				else if (bestETarget)
				{
					if (castE(bestETarget, "harassbounce")) return;
				}
			}

			if (shouldBreak) break;

		}
	}

	void particleHandling()
	{
		// Store particle settings
		auto particleQ = settings::automatic::qParticle->get_bool() && isQReady;
		auto particleW = settings::automatic::wParticle->get_bool() && isWReady;

		// Checking if particles are valid, if they're not, delete them from the list
		particlePredList.erase(std::remove_if(particlePredList.begin(), particlePredList.end(), [](const particleStruct& x)
			{
				return !x.obj->is_valid() || x.owner->is_dead() || x.time + x.castTime <= gametime->get_time();
			}
		),
		particlePredList.end());

		if (hasCasted || (!particleQ && !particleW)) return;

		// Loop through every pred particles
		for (auto& obj : particlePredList)
		{
			// Getting the final cast position
			if (obj.isTeleport)
			{
				obj.target = obj.obj->get_particle_attachment_object();
				if (!obj.target)
					obj.target = obj.obj->get_particle_target_attachment_object();
				if (obj.target && obj.obj->get_position().distance(obj.target->get_position()) <= 0) {
					obj.castingPos = obj.target->get_position().extend(nexusPos, obj.target->get_bounding_radius() + obj.owner->get_bounding_radius());
				}
				else
				{
					obj.castingPos = obj.obj->get_position().extend(nexusPos, obj.owner->get_bounding_radius());
				}
				if (obj.castingPos.is_wall() || obj.castingPos.is_building())
					obj.castingPos = obj.obj->get_position();
			}
			else if (obj.isZed)
			{
				obj.castingPos = obj.target->get_position() + (obj.owner->get_direction() * (obj.target->get_bounding_radius() + obj.owner->get_bounding_radius()));
				if (obj.castingPos.is_wall() || obj.castingPos.is_building())
					obj.castingPos = obj.target->get_position() + (obj.owner->get_direction() * 1);
			}

			// Check if cast position isn't too far enough
			if ((myhero->get_position().distance(obj.castingPos) - obj.owner->get_bounding_radius()) > BRAND_Q_RANGE) continue;

			// Gathering enough data to cast on particles
			const auto& distance = myhero->get_position().distance(obj.castingPos) - (obj.owner->get_bounding_radius());
			const auto& qLandingTime = std::max(q->get_delay(), (distance / q->get_speed()) + q->get_delay());
			const auto& particleTime = (obj.time + obj.castTime) - gametime->get_time();
			const auto& qCanDodge = obj.owner->get_move_speed() * ((qLandingTime - particleTime) + getPing()) > q->get_radius() + obj.owner->get_bounding_radius();
			const auto& wCanDodge = obj.owner->get_move_speed() * ((w->get_delay() - particleTime) + getPing()) > w->get_radius();
			const auto& collisionList = q->get_collision(myhero->get_position(), { obj.castingPos });
			const auto& canQ = particleQ && !qCanDodge && timeBeforeWHitsLocation(obj.castingPos) < FLT_MAX && collisionList.empty();
			const auto& canW = particleW && !wCanDodge && myhero->get_position().distance(obj.castingPos) <= w->range();

			// Try to cast Q if possible
			if (canQ && (particleTime - getPing() + 0.2 <= qLandingTime))
			{
				q->cast(obj.castingPos);
				hasCasted = true;
				return;
			}
			// Try to cast W if possible
			else if (canW && (particleTime - getPing() + 0.1) <= w->get_delay())
			{
				w->cast(obj.castingPos);
				hasCasted = true;
				return;
			}
		}
	}

	void automatic()
	{
		// Check if you didn't already cast
		if (hasCasted || (settings::automatic::towerCheck->get_bool() && isUnderTower(myhero))) return;

		// Store every settings
		const auto& ccQ = settings::automatic::qStun->get_bool() && isQReady;
		const auto& ccW = settings::automatic::wStun->get_bool() && isWReady;
		const auto& dashQ = settings::automatic::qDash->get_bool() && isQReady;
		const auto& dashW = settings::automatic::wDash->get_bool() && isWReady;
		const auto& castingQ = settings::automatic::qCast->get_bool() && isQReady;
		const auto& castingW = settings::automatic::wCast->get_bool() && isWReady;
		const auto& channelQ = settings::automatic::qChannel->get_bool() && isQReady;
		const auto& channelW = settings::automatic::wChannel->get_bool() && isWReady;
		const auto& stasisQ = settings::automatic::qStasis->get_bool() && isQReady;
		const auto& stasisW = settings::automatic::wStasis->get_bool() && isWReady;

		// Stop if player doesn't want to use any auto stuff
		if (!ccQ && !ccW && !dashQ && !dashW && !castingQ && !castingW && !channelQ && !channelW && !stasisQ && !stasisW && particlePredList.empty()) return;

		// Loop through every sorted targets
		for (const auto& target : targets)
		{
			const auto& stasisDuration = stasisInfo[target->get_handle()].stasisTime;
			// Valid target check
			const bool& isValidTarget = target && (customIsValid(target) || stasisDuration > 0) && !target->is_zombie();
			// If not valid then go to next target
			if (!isValidTarget) continue;

			const auto& dashing = target->is_dashing() || qPredictionList[target->get_handle()].hitchance == hit_chance::dashing || wPredictionList[target->get_handle()].hitchance == hit_chance::dashing;
			const auto& ccTime = stunTime[target->get_handle()];
			const auto& channelingSpell = (target->is_casting_interruptible_spell() >= 1 || isRecalling(target)) && !isCastMoving(target);
			const auto& ccCast = ccTime > 0 && (ccQ || ccW);
			const auto& dashingCast = dashing && (dashQ || dashW);
			const auto& castingSpell = target->get_active_spell() && target->get_active_spell()->cast_start_time() - 0.033 >= gametime->get_time();
			const auto& castingCast = castingSpell && !target->get_active_spell()->get_spell_data()->is_insta() && !target->get_active_spell()->get_spell_data()->mCanMoveWhileChanneling() && (castingQ || castingW) && !isCastMoving(target);
			const auto& channelingCast = channelingSpell && (channelQ || channelW);
			const auto& stasisCast = stasisDuration > 0 && (stasisQ || stasisW);
			if (!ccCast && !dashingCast && !castingCast && !channelingCast && !stasisCast) continue;

			auto& qp = qPredictionList[target->get_handle()];
			const auto& qLandingTime = getTimeToHit(qp.input, qp, true);
			const auto& canE = isEReady && target->get_position().distance(myhero->get_position()) <= BRAND_E_RANGE;

			// Cast on stasis targets
			if (stasisCast)
			{
				// Cast Q on stasis
				if (stasisQ && (stasisDuration + 0.133) < qLandingTime && castQ(target, "stasis", false, true)) break;
				// Cast W on stasis
				if (stasisW && (stasisDuration + 0.2 - getPing()) < w->get_delay() && castW(target, "stasis", true)) break;
			}

			// Next part shouldn't cast on stasis targets
			if (stasisDuration > 0) continue;

			// Cast on stun logic
			if (ccCast) {
				// Cast Q on stun
				if (ccQ)
				{
					// Chain CC logic
					if ((ccTime - getPing()) >= qLandingTime || !castQ(target, "stun", canE))
					{
						if (isQReady && canE && castE(target, "stun")) break;
					}
					else
						break;
				}
				// Cast W on stun
				if (ccW && castW(target, "stun")) break;
			}

			// Cast on dash logic
			if (dashingCast) {
				// Cast Q on dash
				if (dashQ)
				{
					if (!castQ(target, "dash", canE))
					{
						if (isQReady && canE && castE(target, "dash")) break;
					}
					else
						break;
				}
				// Cast W on dash
				if (dashW && castW(target, "dash")) break;
			}

			// Cast on casting logic
			if (castingCast) {
				// Cast Q on casting
				if (castingQ)
				{
					if (!castQ(target, "casting", canE))
					{
						if (isQReady && canE && castE(target, "casting")) break;
					}
					else
						break;
				}
				// Cast W on casting
				if (castingW && castW(target, "casting")) break;
			}

			// Cast on channeling logic
			if (channelingCast) {
				// Cast Q on channel
				if (channelQ)
				{
					if (!castQ(target, "channeling", canE))
					{
						if (isQReady && canE && castE(target, "channeling")) break;
					}
					else
						break;
				}
				// Cast W on channel
				if (channelW && castW(target, "channeling")) break;
			}

		}

		// Particle pred handling
		if (!particlePredList.empty())
			particleHandling();

	}

	void createMenu()
	{
		// Main tab
		mainMenu = menu->create_tab("open.brand", "[AURORA] OpenBrand");
		mainMenu->set_texture(myhero->get_square_icon_portrait());

		// Combo settings
		const auto comboTab = mainMenu->add_tab("open.brand.combo", "Combo");
		settings::combo::qCombo = comboTab->add_checkbox("open.brand.combo.qcombo", "Q combo", true);
		settings::combo::qCombo->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
		settings::combo::wCombo = comboTab->add_checkbox("open.brand.combo.wcombo", "W combo", true);
		settings::combo::wCombo->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
		settings::combo::eCombo = comboTab->add_checkbox("open.brand.combo.ecombo", "E combo", true);
		settings::combo::eCombo->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
		settings::combo::eExtraCombo = comboTab->add_checkbox("open.brand.combo.eextracombo", "^ Cast on minions to hit champions", true);
		settings::combo::rCombo = comboTab->add_checkbox("open.brand.combo.rcombo", "R combo", true);
		settings::combo::rCombo->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
		settings::combo::rComboAoE = comboTab->add_checkbox("open.brand.combo.rcomboaoe", "^ AoE cast?", true);
		settings::combo::rComboAoE->add_property_change_callback([](TreeEntry* entry){
				if (entry->get_bool())
				{
					settings::combo::rComboAoEAmount->is_hidden() = false;
				}
				else
				{
					settings::combo::rComboAoEAmount->is_hidden() = true;
				}
			}
		);
		settings::combo::rComboAoEAmount = comboTab->add_slider("open.brand.combo.rcomboaoeamount", "R AoE amount", 2, 2, 5);
		if (!settings::combo::rComboAoE->get_bool())
		{
			settings::combo::rComboAoEAmount->is_hidden() = true;
		}
		settings::combo::rComboKills = comboTab->add_checkbox("open.brand.combo.rcombokills", "^ If killable", false);
		settings::combo::rComboKills->add_property_change_callback([](TreeEntry* entry) {
			if (entry->get_bool())
			{
				settings::combo::rComboLogic->is_hidden() = false;
			}
			else
			{
				settings::combo::rComboLogic->is_hidden() = true;
			}
			}
		);
		settings::combo::rComboLogic = comboTab->add_checkbox("open.brand.combo.rcombologic", "^ Try to avoid wasting", true);
		settings::combo::rComboKills->add_property_change_callback([](TreeEntry* entry) {
			if (entry->get_bool())
			{
				settings::combo::rComboMinionBounce->is_hidden() = false;
			}
			else
			{
				settings::combo::rComboMinionBounce->is_hidden() = true;
			}
			}
		);
		settings::combo::rComboMinionBounce = comboTab->add_checkbox("open.brand.combo.rcombominionbounce", "^ Cast on minions to kill champions", true);
		settings::combo::rComboKills->add_property_change_callback([](TreeEntry* entry) {
				if (entry->get_bool())
				{
					settings::combo::rComboBounces->is_hidden() = false;
				}
				else
				{
					settings::combo::rComboBounces->is_hidden() = true;
				}
			}
		);
		settings::combo::rComboBounces = comboTab->add_slider("open.brand.combo.rcombobounces", "Bounces to kill", 2, 1, 3);
		if (!settings::combo::rComboKills->get_bool())
		{
			settings::combo::rComboLogic->is_hidden() = true;
			settings::combo::rComboMinionBounce->is_hidden() = true;
			settings::combo::rComboBounces->is_hidden() = true;
		}

		// Harass settings
		const auto harassTab = mainMenu->add_tab("open.brand.harass", "Harass");
		settings::harass::qHarass = harassTab->add_checkbox("open.brand.harass.qharass", "Q harass", true);
		settings::harass::qHarass->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
		settings::harass::wHarass = harassTab->add_checkbox("open.brand.harass.wharass", "W harass", true);
		settings::harass::wHarass->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
		settings::harass::eHarass = harassTab->add_checkbox("open.brand.harass.eharass", "E harass", true);
		settings::harass::eHarass->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
		settings::harass::eExtraHarass = harassTab->add_checkbox("open.brand.harass.eextraharass", "^ Cast on minions to hit champions", true);

		// Draw settings
		const auto drawTab = mainMenu->add_tab("open.brand.draws", "Drawings");

		// Draw spellrange tab
		const auto drawRangeTab = drawTab->add_tab("open.brand.draws.ranges", "Spell ranges");
		settings::draws::spellRanges::qRange = drawRangeTab->add_checkbox("open.brand.draws.ranges.qrange", "Draw Q range", true);
		settings::draws::spellRanges::qRange->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
		settings::draws::spellRanges::wRange = drawRangeTab->add_checkbox("open.brand.draws.ranges.wrange", "Draw W range", true);
		settings::draws::spellRanges::wRange->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
		settings::draws::spellRanges::eRange = drawRangeTab->add_checkbox("open.brand.draws.ranges.erange", "Draw E range", true);
		settings::draws::spellRanges::eRange->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
		settings::draws::spellRanges::eExtraRange = drawRangeTab->add_checkbox("open.brand.draws.ranges.eextrarange", "^ Draw bounce range", true);
		settings::draws::spellRanges::rRange = drawRangeTab->add_checkbox("open.brand.draws.rrange", "Draw R range", true);
		settings::draws::spellRanges::rRange->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
		settings::draws::spellRanges::legCircles = drawRangeTab->add_checkbox("open.brand.draws.ranges.legcircles", "LegSense circles", false);

		// Normal draws
		settings::draws::wRadius = drawTab->add_checkbox("open.brand.draws.wradius", "Draw W on ground", true);
		settings::draws::rDamage = drawTab->add_checkbox("open.brand.draws.rdamage", "Draw R damage", true);
		settings::draws::rDamageText = drawTab->add_checkbox("open.brand.draws.rdamagetext", "Draw R damage text", true);
		settings::draws::particlePos = drawTab->add_checkbox("open.brand.draws.particlepos", "Draw particle pred positions", false);
		settings::draws::particlePos->set_tooltip("Keep it disabled if you use OpenUtilities teleport tracker");
		settings::draws::stasisPos = drawTab->add_checkbox("open.brand.draws.stasispos", "Draw stasis pred positions", true);

		// Hitchance tab
		const auto hitchanceTab = mainMenu->add_tab("open.band.hitchance", "Hitchance");
		std::vector<std::pair<std::string, void*>> combo_elements = {};
		for (int i = 3; i < 9; i++)
		{
			const auto hitchance = static_cast<hit_chance>(i);

			std::string hitchance_str;
			switch (hitchance)
			{
			case hit_chance::immobile:
				hitchance_str = "Immobile";
				break;
			case hit_chance::dashing:
				hitchance_str = "Dashing";
				break;
			case hit_chance::very_high:
				hitchance_str = "Very High";
				break;
			case hit_chance::high:
				hitchance_str = "High";
				break;
			case hit_chance::medium:
				hitchance_str = "Medium";
				break;
			case hit_chance::low:
				hitchance_str = "Low";
				break;
			case hit_chance::impossible:
				hitchance_str = "Impossible";
				break;
			case hit_chance::out_of_range:
				hitchance_str = "Out Of Range";
				break;
			case hit_chance::collision:
				hitchance_str = "Collision";
				break;
			}

			combo_elements.emplace_back(hitchance_str, nullptr);
		}
		settings::hitchance::qHitchance = hitchanceTab->add_combobox("open.brand.hitchance.qhitchance", "Q Hitchance", combo_elements, 2);
		settings::hitchance::wHitchance = hitchanceTab->add_combobox("open.brand.hitchance.whitchance", "W Hitchance", combo_elements, 2);

		// Misc tab
		const auto miscTab = mainMenu->add_tab("open.brand.misc", "Misc");
		settings::automatic::towerCheck = miscTab->add_checkbox("open.brand.misc.towercheck", "Don't auto cast under turret", false);
		settings::automatic::attackCheck = miscTab->add_checkbox("open.brand.misc.attackcheck", "Don't cancel auto to cast", false);
		settings::automatic::fowPred = miscTab->add_checkbox("open.brand.misc.fowpred", "AuroraPred FoW prediction", true);
		settings::automatic::qeLogic = miscTab->add_checkbox("open.brand.misc.qelogic", "Try to Q-E", false);
		settings::automatic::qStun = miscTab->add_checkbox("open.brand.misc.qstun", "Auto Q on stun", true);
		settings::automatic::wStun = miscTab->add_checkbox("open.brand.misc.wstun", "Auto W on stun", true);
		settings::automatic::qDash = miscTab->add_checkbox("open.brand.misc.qdash", "Auto Q on dash", true);
		settings::automatic::wDash = miscTab->add_checkbox("open.brand.misc.wdash", "Auto W on dash", true);
		settings::automatic::qCast = miscTab->add_checkbox("open.brand.misc.qcast", "Auto Q on cast", true);
		settings::automatic::wCast = miscTab->add_checkbox("open.brand.misc.wcast", "Auto W on cast", true);
		settings::automatic::qChannel = miscTab->add_checkbox("open.brand.misc.qchannel", "Auto Q on channel", true);
		settings::automatic::wChannel = miscTab->add_checkbox("open.brand.misc.wchannel", "Auto W on channel", true);
		settings::automatic::qStasis = miscTab->add_checkbox("open.brand.misc.qstasis", "Auto Q on stasis", true);
		settings::automatic::wStasis = miscTab->add_checkbox("open.brand.misc.wstasis", "Auto W on stasis", true);
		settings::automatic::qParticle = miscTab->add_checkbox("open.brand.misc.qparticle", "Auto Q on particle", true);
		settings::automatic::wParticle = miscTab->add_checkbox("open.brand.misc.wparticle", "Auto W on particle", true);

		// Misc
		settings::lowSpec = mainMenu->add_checkbox("open.brand.lowspec", "Low spec mode (tick limiter)", false);
		settings::debugPrint = mainMenu->add_checkbox("open.brand.debugprint", "Debug print in console (dev)", false);
	}

	void on_update()
	{
		// Limit ticks (for low spec mode)
		if (settings::lowSpec->get_bool() && limitedTick(SERVER_TICKRATE)) return;

		// Sort targets
		targetSelectorSort();

		// Pred, damage && other calcs needed for many things
		calcs();

		// E spam because Roti Gemes
		if (eSpam()) return;

		// Check if player can cast spells
		if (!canCastSpells()) return;

		// Combo mode
		combo();

		// Harass mode
		harass();

		// Auto cast
		automatic();

	}

	void on_draw()
	{
		// Spellranges
		
		// Q
		if (settings::draws::spellRanges::qRange->get_bool()) {
			const auto& alpha = isQReady ? 255 : 50;
			drawCircle(myhero->get_position(), BRAND_Q_RANGE, 100, settings::draws::spellRanges::legCircles->get_bool(), MAKE_COLOR(204, 127, 0, alpha), 2);
		}

		// W
		if (settings::draws::spellRanges::wRange->get_bool()) {
			const auto& alpha = isWReady ? 255 : 50;
			drawCircle(myhero->get_position(), BRAND_W_RANGE, 100, settings::draws::spellRanges::legCircles->get_bool(), MAKE_COLOR(255, 0, 0, alpha), 2);
		}

		// E
		if (settings::draws::spellRanges::eRange->get_bool()) {
			const auto& alpha = isEReady ? 255 : 50;
			drawCircle(myhero->get_position(), BRAND_E_RANGE, 100, settings::draws::spellRanges::legCircles->get_bool(), MAKE_COLOR(0, 127, 255, alpha), 2);
		}

		// ExtraE
		if (settings::draws::spellRanges::eExtraRange->get_bool() && isEReady)
		{
			for (const auto& target : eBounceTargets)
			{
				if (!customIsValid(target.target, BRAND_E_RANGE)) continue;

				const auto& isMainTarget = bestETarget && target.target->get_handle() == bestETarget->get_handle();
				drawCircle(target.target->get_position(), target.extraRange ? BRAND_E_MAX_BOUNCE_RANGE : BRAND_E_MIN_BOUNCE_RANGE, 100, settings::draws::spellRanges::legCircles->get_bool(), MAKE_COLOR(0, 127, 255, isMainTarget ? 160 : 75), 2);
			}
		}

		// R
		if (settings::draws::spellRanges::rRange->get_bool()) {
			const auto& alpha = isRReady ? 255 : 50;
			drawCircle(myhero->get_position(), BRAND_R_RANGE, 100, settings::draws::spellRanges::legCircles->get_bool(), MAKE_COLOR(255, 127, 0, alpha), 2);
		}

		// Misc

		// Draw misc
		for (const auto& target : entitylist->get_enemy_heroes())
		{
			if (!target->is_valid()) continue;

			// Draw stasis pred pos
			const auto& stasisData = stasisInfo[target->get_handle()];
			if (settings::draws::stasisPos->get_bool() && stasisData.stasisTime > 0 && stasisData.stasisEnd < gametime->get_time())
			{
				const auto& castTime = stasisData.stasisEnd - stasisData.stasisStart;
				draw_manager->add_filled_circle(target->get_position(), target->get_bounding_radius() * std::min(1.f, (1 / (castTime / (gametime->get_time() - stasisData.stasisStart)))), MAKE_COLOR(255, 127, 0, 64));
			}
		}

		// Draw particle pred positions
		if (settings::draws::particlePos->get_bool())
		{
			for (const auto& obj : particlePredList)
			{
				if (!obj.obj->is_valid() || obj.owner->is_dead() || obj.time + obj.castTime <= gametime->get_time() || obj.castingPos == vector::zero) continue;

				draw_manager->add_filled_circle(obj.castingPos, obj.owner->get_bounding_radius() * std::min(1.f, (1 / (obj.castTime / (gametime->get_time() - obj.time)))), MAKE_COLOR(255, 0, 255, 64));
			}
		}

		// Draw W on ground
		if (settings::draws::wRadius->get_bool()) {
			for (const auto& particle : particleList) {
				draw_manager->add_circle_with_glow(particle.particle->get_position(), MAKE_COLOR(255, 127, 0, 255), w->get_radius() * std::min(1.f, (1 / (BRAND_W_PARTICLE_TIME / (gametime->get_time() - particle.creationTime)))), 2.F, glow_data(1.f, 0.75f, 0.f, 1.f));
			}
		}

	}

	void on_draw_real()
	{
		// Draw W on ground
		if (settings::draws::wRadius->get_bool()) {
			for (const auto& particle : particleList) {
				draw_manager->add_circle_with_glow(particle.particle->get_position(), MAKE_COLOR(255, 0, 0, 255), w->get_radius(), 2.F, glow_data(0.2f, 0.5f, 1.f, 0.5f));
			}
		}

		// Draw R damage & damage text
		for (const auto& target : entitylist->get_enemy_heroes())
		{
			if (!target->is_valid()) continue;

			// Draw stasis pred pos
			const auto& stasisData = stasisInfo[target->get_handle()];
			if (settings::draws::stasisPos->get_bool() && stasisData.stasisTime > 0 && stasisData.stasisEnd < gametime->get_time())
			{
				draw_manager->add_circle(target->get_position(), target->get_bounding_radius(), MAKE_COLOR(255, 255, 0, 255), 2);
				const auto& castTime = stasisData.stasisEnd - stasisData.stasisStart;
				draw_manager->add_circle(target->get_position(), target->get_bounding_radius() * std::min(1.f, (1 / (castTime / (gametime->get_time() - stasisData.stasisStart)))), MAKE_COLOR(255, 127, 0, 255), 2);
			}

			if (!target->is_visible_on_screen() || !target->is_hpbar_recently_rendered() || target->is_dead() || isYuumiAttached(target)) continue;

			if (rDamageList[target->get_handle()].damage > 0) {
				if (settings::draws::rDamage->get_bool()) {
					draw_dmg_rl(target, rDamageList[target->get_handle()].damage, MAKE_COLOR(255, 170, 0, 150));
				}
				if (settings::draws::rDamageText->get_bool()) {
					auto bar_pos = target->get_hpbar_pos();
					bar_pos = vector(bar_pos.x + 130, bar_pos.y -= 40);
					if (rDamageList[target->get_handle()].kills)
					{
						draw_manager->add_text_on_screen(bar_pos, D3DCOLOR_ARGB(255, 255, 0, 0), 20, "Killable -> %d %s", rDamageList[target->get_handle()].shots, rDamageList[target->get_handle()].shots > 1 ? "bounces" : "bounce");
					}
					else
					{
						const auto& damagePercent = (rDamageList[target->get_handle()].damage / target->get_health());
						const int& red = std::round(damagePercent * 255);
						draw_manager->add_text_on_screen(bar_pos, D3DCOLOR_ARGB(255, red, 255 - red, 255 - red), 20, "%.0f (%i%%)", std::round(getTotalHP(target) - rDamageList[target->get_handle()].damage), (int)std::round(damagePercent * 100));
					}
				}
			}
		}

		// Draw particle pred positions
		if (settings::draws::particlePos->get_bool())
		{
			for (const auto& obj : particlePredList)
			{
				if (!obj.obj->is_valid() || obj.owner->is_dead() || obj.time + obj.castTime <= gametime->get_time() || obj.castingPos == vector::zero) continue;

				draw_manager->add_circle(obj.castingPos, obj.owner->get_bounding_radius(), MAKE_COLOR(138, 43, 226, 255), 2);
				draw_manager->add_circle(obj.castingPos, obj.owner->get_bounding_radius() * std::min(1.f, (1 / (obj.castTime / (gametime->get_time() - obj.time)))), MAKE_COLOR(255, 0, 255, 255), 2);
				vector screenPos;
				renderer->world_to_screen(obj.castingPos, screenPos);
				const auto size = vector(30.f, 30.f);
				const auto sizeMod = size / 2;
				draw_manager->add_image(obj.owner->get_square_icon_portrait(), { screenPos.x - sizeMod.x, screenPos.y - sizeMod.y }, size);
			}
		}
	}
	
	void on_create(const game_object_script obj)
	{
		// Get object name hash
		const auto& object_hash = spell_hash_real(obj->get_name_cstr());

		// Get emitter hash
		const auto& emitterHash = obj->get_emitter_resources_hash();

		// Get Brand W particle & store it
		if (emitterHash == buff_hash("Brand_W_POF_charge") && obj->get_emitter() && obj->get_emitter()->is_ally())
		{
			particleList.push_back({ .particle = obj, .creationTime = gametime->get_time() });
			return;
		}

		// Get particles to cast on
		if (!obj->get_emitter() || !obj->get_emitter()->is_enemy() || !obj->get_emitter()->is_ai_hero()) return;

		switch (emitterHash)
		{
			case buff_hash("TwistedFate_R_Gatemarker_Red"):
			{
				const particleStruct& particleData = { .obj = obj, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 1.5, .castingPos = obj->get_position() };
				particlePredList.push_back(particleData);
				return;
			}
			case buff_hash("Ekko_R_ChargeIndicator"):
			{
				const particleStruct& particleData = { .obj = obj, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 0.5, .castingPos = obj->get_position() };
				particlePredList.push_back(particleData);
				return;
			}
			case buff_hash("Pantheon_R_Update_Indicator_Enemy"):
			{
				const auto& castPos = obj->get_position() + obj->get_particle_rotation_forward() * 1350;
				const particleStruct& particleData = { .obj = obj, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 2.2, .castingPos = castPos };
				particlePredList.push_back(particleData);
				return;
			}
			case buff_hash("Galio_R_Tar_Ground_Enemy"):
			{
				const particleStruct& particleData = { .obj = obj, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 2.75, .castingPos = obj->get_position() };
				particlePredList.push_back(particleData);
				return;
			}
			case buff_hash("Evelynn_R_Landing"):
			{
				const particleStruct& particleData = { .obj = obj, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 0.85, .castingPos = obj->get_position() };
				particlePredList.push_back(particleData);
				return;
			}
			case buff_hash("TahmKench_W_ImpactWarning_Enemy"):
			{
				const particleStruct& particleData = { .obj = obj, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 0.8, .castingPos = obj->get_position() };
				particlePredList.push_back(particleData);
				return;
			}
			case buff_hash("Zed_R_tar_TargetMarker"):
			if (obj->get_particle_attachment_object() && obj->get_particle_attachment_object()->get_handle() == myhero->get_handle()) {
				const particleStruct& particleData = { .obj = obj, .target = obj->get_particle_attachment_object(), .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 0.95, .castingPos = vector::zero, .isZed = true };
				particlePredList.push_back(particleData);
				return;
			}
			case 1882371666:
			{
				const particleStruct& particleData = { .obj = obj, .target = obj->get_particle_attachment_object(), .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = obj->get_position().distance(urfCannon) / 2800, .castingPos = obj->get_position() };
				particlePredList.push_back(particleData);
				return;
			}
		}

		if (obj->get_emitter()->get_teleport_state() != "SummonerTeleport") return;

		if (object_hash == spell_hash("global_ss_teleport_turret_red.troy"))
		{
			const auto& target = obj->get_particle_attachment_object();
			if (nexusPos != vector::zero)
			{
				const particleStruct& particleData = { .obj = obj, .target = target, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 4.1, .castingPos = vector::zero, .isTeleport = true };
				particlePredList.push_back(particleData);
				return;
			}
		}
		else if (object_hash == spell_hash("global_ss_teleport_target_red.troy"))
		{
			const auto& target = obj->get_particle_target_attachment_object();
			if (nexusPos != vector::zero)
			{
				const particleStruct& particleData = { .obj = obj, .target = target, .owner = obj->get_emitter(), .time = gametime->get_time(), .castTime = 4.1, .castingPos = vector::zero, .isTeleport = true };
				particlePredList.push_back(particleData);
				return;
			}
		}
	}

	void on_delete(const game_object_script obj)
	{
		// Delete Brand W in list when a W particle gets deleted with handle comparison
		if (obj->get_emitter_resources_hash() == buff_hash("Brand_W_POF_charge") && obj->get_emitter() && obj->get_emitter()->is_ally())
			particleList.erase(std::remove_if(particleList.begin(), particleList.end(), [obj](particleData& particle)
			{
				return particle.particle->get_handle() == obj->get_handle();
			}), particleList.end());
	}

	void on_buff(game_object_script& sender, buff_instance_script& buff, const bool gain)
	{
		if (!sender || !buff) return;
		
		// Detects if someone is reviving from Guardian Angel
		if (!gain && buff->get_hash_name() == buff_hash("willrevive") && sender->is_playing_animation(buff_hash("Death")) && sender->has_item(ItemId::Guardian_Angel) != spellslot::invalid)
		{
			guardianReviveTime[sender->get_handle()] = deathAnimTime[sender->get_handle()] + 4;
			return;
		}
	}

	void on_buff_gain(game_object_script sender, buff_instance_script buff)
	{
		// Grouping on buff gain && on buff lose together
		on_buff(sender, buff, true);
	}

	void on_buff_lose(game_object_script sender, buff_instance_script buff)
	{
		// Grouping on buff gain && on buff lose together
		on_buff(sender, buff, false);
	}

	void on_cast_spell(spellslot spellSlot, game_object_script target, vector& pos, vector& pos2, bool isCharge, bool* process)
	{
		lastCast = gametime->get_time() + 0.133 + getPing();
	}

	void on_process_spell_cast(game_object_script sender, spell_instance_script spell)
	{
		if (sender->get_handle() == myhero->get_handle()) lastCast = 0;
	}

	void on_network_packet(game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args)
	{
		if (type != pkttype_e::PKT_S2C_PlayAnimation_s || !sender) return;

		const auto& data = (PKT_S2C_PlayAnimationArgs*)args;
		if (!data) return;

		if (strcmp(data->animation_name, "Death") == 0)
		{
			deathAnimTime[sender->get_handle()] = gametime->get_time();
		}
	}

	void load()
	{
		// Spell registering
		
		// Q
		q = plugin_sdk->register_spell(spellslot::q, BRAND_Q_RANGE);
		q->set_skillshot(0.25f, 60.f, 1600.f, { collisionable_objects::minions, collisionable_objects::heroes, collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);
		q->set_spell_lock(false);

		// W
		w = plugin_sdk->register_spell(spellslot::w, BRAND_W_RANGE);
		w->set_skillshot(0.95f, 260.f, FLT_MAX, {}, skillshot_type::skillshot_circle);
		w->set_spell_lock(false);

		// E
		e = plugin_sdk->register_spell(spellslot::e, BRAND_E_RANGE);
		e->set_skillshot(0.25f, 30.f, FLT_MAX, { collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);
		e->set_spell_lock(false);

		// R
		r = plugin_sdk->register_spell(spellslot::r, BRAND_R_RANGE);
		r->set_skillshot(0.f, 60.f, FLT_MAX, { collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);
		r->set_spell_lock(false);

		// Get enemy Nexus pos

		const auto& nexusPosIt = std::find_if(entitylist->get_all_nexus().begin(), entitylist->get_all_nexus().end(), [](game_object_script x) {
			return x->is_enemy();
			}
		);
		const auto& nexusEntity = *nexusPosIt;
		nexusPos = nexusEntity->get_position();

		// Get URF cannon pos
		urfCannon = myhero->get_team() == game_object_team::order ? vector(13018.f, 14026.f) : vector(1506.f, 676.f);

		// Call menu creation function
		createMenu();

		// Warning if trolling
		scheduler->delay_action(0.1f, []()
			{
				aurora_prediction = menu->get_tab("aurora_prediction");
				if (!aurora_prediction || aurora_prediction->is_hidden() != false)
				{
					myhero->print_chat(0, "<font color=\"#2dce89\">[OpenSeries]</font> <font color=\"#fd5d93\">Load and select Aurora Prediction for better performance !</font>");
				}
			}
		);

		// Add events
		event_handler<events::on_update>::add_callback(on_update);
		event_handler<events::on_env_draw>::add_callback(on_draw);
		event_handler<events::on_draw>::add_callback(on_draw_real);
		event_handler<events::on_create_object>::add_callback(on_create);
		event_handler<events::on_delete_object>::add_callback(on_delete);
		event_handler<events::on_buff_gain>::add_callback(on_buff_gain);
		event_handler<events::on_buff_lose>::add_callback(on_buff_lose);
		event_handler<events::on_cast_spell>::add_callback(on_cast_spell);
		event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);
		event_handler<events::on_network_packet>::add_callback(on_network_packet);

	}

	void unload()
	{
		// Remove events
		event_handler< events::on_update >::remove_handler(on_update);
		event_handler< events::on_env_draw >::remove_handler(on_draw);
		event_handler< events::on_draw >::remove_handler(on_draw_real);
		event_handler< events::on_create_object >::remove_handler(on_create);
		event_handler< events::on_delete_object >::remove_handler(on_delete);
		event_handler< events::on_buff_gain >::remove_handler(on_buff_gain);
		event_handler< events::on_buff_lose >::remove_handler(on_buff_lose);
		event_handler< events::on_cast_spell >::remove_handler(on_cast_spell);
		event_handler< events::on_process_spell_cast >::remove_handler(on_process_spell_cast);
		event_handler< events::on_network_packet >::remove_handler(on_network_packet);
	}

}
