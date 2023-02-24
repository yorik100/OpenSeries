#include "xerath.h"
#include <unordered_set>
#include <unordered_map>

namespace xerath {

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
		float priority = 0;
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
	std::vector<game_object_script> eMissileList;
	std::vector<particleData> particleList;
	std::vector<particleData> ultParticleList;
	std::vector<eBounceTarget> eBounceTargets;
	std::vector<particleStruct> particlePredList;

	std::unordered_map<uint32_t, prediction_output> qPredictionList;
	std::unordered_map<uint32_t, prediction_output> wPredictionList;
	std::unordered_map<uint32_t, prediction_output> w2PredictionList;
	std::unordered_map<uint32_t, prediction_output> ePredictionList;
	std::unordered_map<uint32_t, prediction_output> rPredictionList;
	std::unordered_map<uint32_t, prediction_output> qDummyPredictionList;
	std::unordered_map<uint32_t, prediction_output> q2PredictionList;
	std::unordered_map<uint32_t, stasisStruct> stasisInfo;
	std::unordered_map<uint32_t, float> stunTime;
	std::unordered_map<uint32_t, float> guardianReviveTime;
	std::unordered_map<uint32_t, float> godBuffTime;
	std::unordered_map<uint32_t, float> noKillBuffTime;
	std::unordered_map<uint32_t, float> qDamageList;
	std::unordered_map<uint32_t, float> wDamageList;
	std::unordered_map<uint32_t, float> eDamageList;
	std::unordered_map<uint32_t, rDamageData> rDamageList;

	static std::unordered_set godBuffList{
		buff_hash("KayleR"),
		buff_hash("TaricR"),
		buff_hash("SivirE"),
		buff_hash("FioraW"),
		buff_hash("NocturneShroudofDarkness"),
		buff_hash("kindredrnodeathbuff"),
		buff_hash("XinZhaoRRangedImmunity"),
		buff_hash("PantheonE")
	};

	static std::unordered_set noKillBuffList{
		buff_hash("UndyingRage"),
		buff_hash("ChronoShift")
	};

	static std::unordered_set stasisBuffList{
		buff_hash("ChronoRevive"),
		buff_hash("BardRStasis"),
		buff_hash("ZhonyasRingShield"),
		buff_hash("LissandraRSelf")
	};

	static std::unordered_set immuneSpells = {
		spell_hash("EvelynnR"),
		spell_hash("ZedR"),
		spell_hash("EkkoR"),
		spell_hash("FizzE"),
		spell_hash("FizzETwo"),
		spell_hash("FizzEBuffer"),
		spell_hash("XayahR"),
		spell_hash("VladimirSanguinePool")
	};

	static std::unordered_set ignoreSpells = {
		spell_hash("NunuW"),
		spell_hash("SionR")
	};

	script_spell* q;
	script_spell* w;
	script_spell* e;
	script_spell* r;
	script_spell* qCharge;
	script_spell* q2;

	TreeTab* mainMenu;
	namespace settings {
		namespace draws {
			TreeEntry* wRadius;
			TreeEntry* rRadius;
			TreeEntry* rDamage;
			TreeEntry* rDamageText;
			TreeEntry* particlePos;
			TreeEntry* stasisPos;
			TreeEntry* qIndicator;
			TreeEntry* rIndicator;
			TreeEntry* rKillList;
			TreeEntry* rKillListRangeIgnore;
			TreeEntry* killListYOffset;
			TreeEntry* killListXOffset;
			namespace spellRanges {
				TreeEntry* qRange;
				TreeEntry* wRange;
				TreeEntry* eRange;
				TreeEntry* rRange;
				TreeEntry* rMinimapRange;
				TreeEntry* rNearMouseRange;
			}
		}
		namespace combo {
			TreeEntry* qCombo;
			TreeEntry* wCombo;
			TreeEntry* wComboCenter;
			TreeEntry* eCombo;
			TreeEntry* rCombo;
		}
		namespace harass {
			TreeEntry* qHarass;
			TreeEntry* wHarass;
			TreeEntry* wHarassCenter;
			TreeEntry* eHarass;
		}
		namespace automatic {
			TreeEntry* manualEKey;
			TreeEntry* manualRKey;
			TreeEntry* rRange;
			TreeEntry* manualREnable;
			TreeEntry* avoidShields;
			TreeEntry* eStun;
			TreeEntry* wStun;
			TreeEntry* eDash;
			TreeEntry* wDash;
			TreeEntry* qDash;
			TreeEntry* eCast;
			TreeEntry* wCast;
			TreeEntry* qCast;
			TreeEntry* eChannel;
			TreeEntry* wChannel;
			TreeEntry* eStasis;
			TreeEntry* wStasis;
			TreeEntry* qStasis;
			TreeEntry* eParticle;
			TreeEntry* wParticle;
			TreeEntry* qParticle;
		}
		namespace ultimate {
			TreeEntry* rImmobile;
			TreeEntry* rCantDodge;
			TreeEntry* rDash;
			TreeEntry* rCast;
			TreeEntry* rStasis;
		}
		namespace hitchance {
			TreeEntry* qHitchance;
			TreeEntry* wHitchance;
			TreeEntry* eHitchance;
			TreeEntry* rHitchance;
		}
		TreeEntry* lowSpec;
	}

	static constexpr float SERVER_TICKRATE = 1000.f / 30.f;
	static constexpr float XERATH_W_PARTICLE_TIME = 0.8f;
	static constexpr float XERATH_W_OUTER_RADIUS = 275.f;
	static constexpr float XERATH_W_INNER_RADIUS = 100.f;
	static constexpr float XERATH_R_PARTICLE_TIME = 0.65f;
	static constexpr float XERATH_MAX_Q_RANGE = 1450;
	static constexpr float XERATH_MIN_Q_RANGE = 700;
	static constexpr float XERATH_W_RANGE = 1000;
	static constexpr float XERATH_E_RANGE = 1050;
	static constexpr float XERATH_R_RANGE = 5000;

	vector nexusPos;
	vector urfCannon;

	buff_instance_script ultBuff;
	buff_instance_script qBuff;
	buff_instance_script elderBuff;

	game_object_script qTarget;
	game_object_script rTarget;

	bool hasCasted = false;
	bool isQReady = false;
	bool isWReady = false;
	bool isEReady = false;
	bool isRReady = false;

	float last_tick = 0;
	float lastCast = 0;
	int rShots = 0;

	bool isMoving(const game_object_script& target)
	{
		return target->get_path_controller() && !target->get_path_controller()->is_dashing() && target->is_moving();
	}

	float timeBeforeWHits(const game_object_script& target)
	{
		// Get time to hit before any W particle hits target (including ally W particles, useful in one for all)
		float returnTimeToHit = FLT_MAX;
		for (const auto& particle : particleList) {
			const auto& timeBeforeHit = particle.creationTime + XERATH_W_PARTICLE_TIME - gametime->get_time();
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
			const auto& timeBeforeHit = particle.creationTime + XERATH_W_PARTICLE_TIME - gametime->get_time();
			const auto& unitPositionDist = position.distance(particle.particle->get_position());
			if (particle.particle->is_valid() && unitPositionDist <= w->get_radius() && returnTimeToHit > timeBeforeHit)
				returnTimeToHit = timeBeforeHit;
		}
		return returnTimeToHit;
	}

	bool willGetHitByE(const game_object_script& target)
	{
		if (!target) return false;
		for (const auto& missile : eMissileList)
		{
			if (!missile) continue;
			e->set_delay(- getPing() - 0.1);
			const auto& eCollisions = e->get_collision(missile->get_position(), {missile->missile_get_end_position()});
			e->set_delay(0.25);
			if (eCollisions.empty()) continue;
			if (eCollisions[0]->get_handle() == target->get_handle()) return true;
		}
		return false;
	}

	bool willGetHitByR(const game_object_script& target)
	{
		for (const auto& particle : ultParticleList) {
			const auto& timeBeforeHit = particle.creationTime + XERATH_R_PARTICLE_TIME - gametime->get_time();
			const auto& unitPositionDist = prediction->get_prediction(target, std::max(0.f, timeBeforeHit)).get_unit_position().distance(particle.particle->get_position());
			if (particle.particle->is_valid() && unitPositionDist <= r->get_radius())
				return true;
		}
		return false;
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
			if (godBuffList.contains(buffHash))
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
			if (noKillBuffList.contains(buffHash))
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
			if (stasisBuffList.contains(buffHash))
			{
				if (buffTime < buff->get_remaining_time())
				{
					buffTime = buff->get_remaining_time();
				}
			}
		}
		// Get guardian angel revive time if there is one
		float GATime = (!target->is_targetable() && guardianReviveTime[target->get_handle()] ? guardianReviveTime[target->get_handle()] - gametime->get_time() : 0);
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
			if (godBuffList.contains(buffHash))
			{
				const auto& isPantheonE = buffHash == buff_hash("PantheonE");
				const auto& realRemainingTime = !isPantheonE ? buff->get_remaining_time() : buff->get_remaining_time() + 0.2;
				if (godBuffTime < realRemainingTime && (!isPantheonE || target->is_facing(myhero)) && (buffHash != buff_hash("XinZhaoRRangedImmunity") || myhero->get_position().distance(target->get_position()) > 450))
				{
					godBuffTime = realRemainingTime;
				}
			}
			else if (noKillBuffList.contains(buffHash))
			{
				if (noKillBuffTime < buff->get_remaining_time())
				{
					noKillBuffTime = buff->get_remaining_time();
				}
			}
			else if (stasisBuffList.contains(buffHash))
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
		float GATime = (!target->is_targetable() && guardianReviveTime[target->get_handle()] ? guardianReviveTime[target->get_handle()] - gametime->get_time() : 0);
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

	hit_chance getPredIntFromSettings(const int hitchance)
	{
		// Get hitchance from settings value
		return static_cast<hit_chance>(hitchance + 3);
	}

	float charged_percentage(const float spell_charge_duration)
	{
		auto buff = qBuff;

		if (buff != nullptr && buff->is_valid() && buff->is_alive())
		{
			return (fmaxf(0.f, fminf(1.f, (gametime->get_time() - buff->get_start() + getPing() + 0.033) / spell_charge_duration)));
		}

		return 0;
	}

	float true_charged_percentage(const float spell_charge_duration)
	{
		auto buff = qBuff;

		if (buff != nullptr && buff->is_valid() && buff->is_alive())
		{
			return (fmaxf(0.f, fminf(1.f, (gametime->get_time() - buff->get_start()) / spell_charge_duration)));
		}

		return 0;
	}

	float charged_range(const float max_range, const float min_range, const float duration)
	{
		if (qBuff)
		{
			return min_range + fminf((float)(max_range - min_range), (float)(max_range - min_range) * charged_percentage(duration));
		}
		return min_range;
	}

	float true_charged_range(const float max_range, const float min_range, const float duration)
	{
		if (qBuff)
		{
			return min_range + fminf((float)(max_range - min_range), (float)(max_range - min_range) * true_charged_percentage(duration));
		}
		return min_range;
	}

	bool castQShort(const game_object_script& target, std::string mode)
	{
		// Cast Q short
		if (hasCasted) return true;

		auto& p = qPredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = q->get_delay() + getPing();
		const auto& trueTimeToHit = q->get_delay();
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (p.hitchance >= getPredIntFromSettings(settings::hitchance::qHitchance->get_int()) && (!willGetHitByE(target) || !isMoving(target)) && aliveWhenLanding && couldDamageLater(target, trueTimeToHit - 0.2, qDamageList[target->get_handle()]))
		{
			q->cast(p.get_cast_position());
			myhero->update_charged_spell(q->get_slot(), p.get_cast_position(), true, true);
			hasCasted = true;
			return true;
		}
		return false;
	}

	bool castQCharge(const game_object_script& target, std::string mode)
	{
		// Cast Q dummy
		if (hasCasted) return true;

		auto& p = qDummyPredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = q->get_delay() + getPing();
		const auto& trueTimeToHit = q->get_delay();
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (p.hitchance > hit_chance::out_of_range && (!willGetHitByE(target) || !isMoving(target)) && aliveWhenLanding && couldDamageLater(target, trueTimeToHit - 0.2, qDamageList[target->get_handle()]))
		{
			qCharge->cast(p.get_cast_position());
			hasCasted = true;
			return true;
		}
		return false;
	}

	bool castQLong(const game_object_script& target, std::string mode)
	{
		// Cast Q charged
		if (hasCasted) return true;

		auto& p = q2PredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = q->get_delay() + getPing();
		const auto& trueTimeToHit = q->get_delay();
		const auto& wTime = timeBeforeWHits(target);
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (p.hitchance >= getPredIntFromSettings(settings::hitchance::qHitchance->get_int()) && aliveWhenLanding && ((!willGetHitByE(target) && wTime >= timeToHit) || !isMoving(target)) && couldDamageLater(target, trueTimeToHit - 0.2, qDamageList[target->get_handle()]))
		{
			myhero->update_charged_spell(q2->get_slot(), p.get_cast_position(), true);
			hasCasted = true;
			return true;
		}
		return false;
	}

	bool castW(const game_object_script& target, std::string mode, bool wCenter = false)
	{
		// Cast W
		if (hasCasted) return true;

		auto& p = !wCenter ? wPredictionList[target->get_handle()] : w2PredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = w->get_delay() + getPing();
		const auto& trueTimeToHit = w->get_delay();
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (p.hitchance >= getPredIntFromSettings(settings::hitchance::wHitchance->get_int()) && (!willGetHitByE(target) || !isMoving(target)) && aliveWhenLanding && couldDamageLater(target, trueTimeToHit - 0.2, wDamageList[target->get_handle()]))
		{
			w->cast(p.get_cast_position());
			hasCasted = true;
			return true;
		}
		return false;
	}

	bool castE(const game_object_script& target, std::string mode)
	{
		// Cast E
		if (hasCasted) return true;

		auto& p = ePredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = getTimeToHit(p.input, p, true);
		const auto& trueTimeToHit = getTimeToHit(p.input, p, false);
		const auto& wTime = timeBeforeWHits(target);
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		if (p.hitchance >= getPredIntFromSettings(settings::hitchance::eHitchance->get_int()) && ((!willGetHitByE(target) && wTime >= timeToHit) || !isMoving(target)) && aliveWhenLanding && couldDamageLater(target, trueTimeToHit - 0.2, eDamageList[target->get_handle()])) {
			e->cast(p.get_cast_position());
			return true;
		}
		return false;
	}

	bool castR(const game_object_script& target, std::string mode)
	{
		// Cast R
		if (hasCasted) return true;

		auto& p = rPredictionList[target->get_handle()];
		if (p.get_cast_position().distance(myhero) > p.input.range) return false;

		const auto& timeToHit = getTimeToHit(p.input, p, true);
		const auto& trueTimeToHit = getTimeToHit(p.input, p, false);
		const auto& aliveWhenLanding = target->get_health() - health_prediction->get_incoming_damage(target, timeToHit + 0.1, true) > 0 || stasisInfo[target->get_handle()].stasisTime > 0;
		const auto& overKill = willGetHitByR(target) && getTotalHP(target) <= getRDamage(target, 0, getTotalHP(target), true);
		if (p.hitchance >= getPredIntFromSettings(settings::hitchance::rHitchance->get_int()) && !overKill && (!willGetHitByE(target) || !isMoving(target)) && aliveWhenLanding && couldDamageLater(target, trueTimeToHit - 0.2, rDamageList[target->get_handle()].damage)) {
			r->cast(p.get_cast_position());
			return true;
		}
		return false;
	}

	prediction_output getQShortPred(const game_object_script& target)
	{
		// Get Q short pred
		q->set_range(XERATH_MIN_Q_RANGE + target->get_bounding_radius());
		const prediction_output& p = q->get_prediction(target);
		return p;
	}

	prediction_output getWPred(const game_object_script& target)
	{
		// Get W pred
		w->set_radius(XERATH_W_OUTER_RADIUS);

		// Delay is randomly higher
		//w->set_delay(isMoving(target) ? 0.9 : XERATH_W_PARTICLE_TIME);

		const prediction_output& p = w->get_prediction(target);
		return p;
	}

	prediction_output getW2Pred(const game_object_script& target)
	{
		// Get W center pred
		w->set_radius(XERATH_W_INNER_RADIUS);

		// Delay is randomly higher
		w->set_delay(isMoving(target) ? 0.9 : XERATH_W_PARTICLE_TIME);

		const prediction_output& p = w->get_prediction(target);
		return p;
	}

	prediction_output getEPred(const game_object_script& target)
	{
		// Get E pred
		const auto& totalRadius = target->get_bounding_radius();
		e->set_range(XERATH_E_RANGE + target->get_bounding_radius());
		e->from = myhero->get_position().distance(target->get_position()) > totalRadius ? myhero->get_position().extend(target->get_position(), totalRadius) : target->get_position();
		prediction_output p = e->get_prediction(target);
		if (p.hitchance <= static_cast<hit_chance>(2)) return p;

		//Behind yourself collision detection
		const auto& collisionsFromHero = e->get_collision(myhero->get_position().extend(target->get_position(), -e->get_radius()), { p.input.get_from() });
		if (!collisionsFromHero.empty()) return prediction_output{};

		return p;
	}

	prediction_output getRPred(const game_object_script& target)
	{
		// Get R pred

		// Delay is randomly higher
		r->set_delay(isMoving(target) ? 0.7 : XERATH_R_PARTICLE_TIME);

		const prediction_output& p = r->get_prediction(target);
		return p;
	}

	prediction_output getQDummyPred(const game_object_script& target)
	{
		// Get Q dummy pred
		qCharge->set_range(XERATH_MAX_Q_RANGE + target->get_bounding_radius());
		const prediction_output& p = qCharge->get_prediction(target);
		return p;
	}
	
	prediction_output getQ2Pred(const game_object_script& target)
	{
		// Get Q charging pred
		
		// Changing charging Q range
		float range = charged_range(XERATH_MAX_Q_RANGE, XERATH_MIN_Q_RANGE, 1.5);
		range = (range < XERATH_MAX_Q_RANGE ? range - 50 : range);
		q2->set_range(range + target->get_bounding_radius());

		const prediction_output& p = q2->get_prediction(target);
		return p;
	}

	bool isValidQ(const game_object_script& target)
	{
		// Get if can cast Q on target
		const auto& range = q2PredictionList[target->get_handle()].input.range;

		// If Q is fully charged or can hit 100% then ignore
		if (range - target->get_bounding_radius() >= XERATH_MAX_Q_RANGE || (q2PredictionList[target->get_handle()].hitchance > hit_chance::very_high && !isMoving(target)))
			return true;

		// If target isn't moving then add extra 50 range to overcharge
		const auto& distance = myhero->get_position().distance(target->get_position());

		// Else add extra range behind to avoid sidestepping by moving backwards
		return range > distance + std::min((int)std::round((target->get_move_speed() * q2->get_delay())), 250);
	}

	float getExtraDamage(const game_object_script& target, const int shots, const float predictedHealth, const float damageDealt, const bool isCC, const bool firstShot, const bool isTargeted)
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
		const float& damage = 30 + spell->level() * 40 + myhero->get_total_ability_power() * 0.85;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, false, true, false);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getQDamageAlternative(const game_object_script& target, const int shots, const float predictedHealth, const int firstShot)
	{
		// Get Q damage
		const auto& spell = myhero->get_spell(spellslot::q);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 30 + spell->level() * 40 + myhero->get_total_ability_power() * 0.85;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, shots, predictedHealth, damageLibDamage, false, firstShot, false);
		if (elderBuff && (predictedHealth - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = getTotalHP(target);
		return totalDamage;
	}

	float getWDamage(const game_object_script& target)
	{
		// Get W normal damage
		const auto& spell = myhero->get_spell(spellslot::w);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 25 + 35 * spell->level() + myhero->get_total_ability_power() * 0.60;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, true, true, false);
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
		const float& damage = (25 + 35 * spell->level() + myhero->get_total_ability_power() * 0.60) * 1.667;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, true, true, false);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getW2DamageAlternative(const game_object_script& target, const int shots, const float predictedHealth, const int firstShot)
	{
		// Get W empowered damage
		const auto& spell = myhero->get_spell(spellslot::w);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = (25 + 35 * spell->level() + myhero->get_total_ability_power() * 0.60) * 1.667;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, shots, predictedHealth, damageLibDamage, true, firstShot, false);
		if (elderBuff && (predictedHealth - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = getTotalHP(target);
		return totalDamage;
	}

	float getEDamage(const game_object_script& target)
	{
		// Get E damage
		const auto& spell = myhero->get_spell(spellslot::e);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 50 + 30 * spell->level() + myhero->get_total_ability_power() * 0.45;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, 0, target->get_health(), damageLibDamage, true, true, false);
		const float& totalHP = getTotalHP(target);
		if (elderBuff && (totalHP - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = totalHP;
		return totalDamage;
	}

	float getEDamageAlternative(const game_object_script& target, const int shots, const float predictedHealth, const int firstShot)
	{
		// Get E damage
		const auto& spell = myhero->get_spell(spellslot::e);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0) return 0;
		const float& damage = 50 + 30 * spell->level() + myhero->get_total_ability_power() * 0.45;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, shots, predictedHealth, damageLibDamage, true, firstShot, false);
		if (elderBuff && (predictedHealth - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = getTotalHP(target);
		return totalDamage;
	}

	float getRDamage(const game_object_script& target, const int shots, const float predictedHealth, const bool firstShot)
	{
		// Get R damage
		const auto& spell = myhero->get_spell(spellslot::r);
		if (spell->level() == 0) return 0;
		if (spell->cooldown() > 0 && ultParticleList.empty() && !ultBuff) return 0;
		const float& damage = 150 + 50 * spell->level() + myhero->get_total_ability_power() * 0.45;
		const float& damageLibDamage = damagelib->calculate_damage_on_unit(myhero, target, damage_type::magical, damage);
		float totalDamage = damageLibDamage + getExtraDamage(target, shots, predictedHealth, damageLibDamage, false, firstShot, false);
		if (elderBuff && (predictedHealth - totalDamage) / target->get_max_health() < 0.2)
			totalDamage = getTotalHP(target);
		return totalDamage;
	}

	rDamageData getTotalRDamage(const game_object_script& target)
	{
		// Get total damage of R & returns damage, shots needed to kill & if it kills
		auto rDamage = 0.f;
		auto shotsToKill = 0;
		auto isFirstShot = true;
		const auto& totalHP = getTotalHP(target);
		const auto& hasUlt = (myhero->get_spell(spellslot::r)->level() != 0 && myhero->get_spell(spellslot::r)->cooldown() <= 0);
		const auto& rActive = hasUlt || !ultParticleList.empty() || ultBuff;
		const auto& shotAmount = ultBuff || !ultParticleList.empty() ? rShots : 2 + myhero->get_spell(spellslot::r)->level();
		if (rActive)
		{
			for (int i = shotAmount; i > 0; i--)
			{
				auto calculatedRDamage = getRDamage(target, i, totalHP - rDamage, isFirstShot);
				auto calculatedRMaxDamage = getRDamage(target, 0, totalHP - rDamage, isFirstShot);
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

		const auto& isCastingImmortalitySpell = ((target->get_active_spell() && immuneSpells.contains(target->get_active_spell()->get_spell_data()->get_name_hash()))) || target->has_buff(buff_hash("AkshanE2"));
		const auto& isValid = !isCastingImmortalitySpell && ((target->is_valid_target(range, from, invul) && target->is_targetable() && target->is_targetable_to_team(myhero->get_team()) && !target->is_invulnerable()));
		return isValid;
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

		// Getting rid of bad W particles
		particleList.erase(std::remove_if(particleList.begin(), particleList.end(), [](const particleData& x)
			{
				return x.creationTime + XERATH_W_PARTICLE_TIME <= gametime->get_time();
			}
		),
			particleList.end());
		// Getting rid of bad R particles
		ultParticleList.erase(std::remove_if(ultParticleList.begin(), ultParticleList.end(), [](const particleData& x)
			{
				return x.creationTime + XERATH_R_PARTICLE_TIME <= gametime->get_time();
			}
		),
			ultParticleList.end());

		// Reset targets
		if (qTarget->is_valid())
			glow->remove_glow(qTarget);
		if (rTarget->is_valid())
			glow->remove_glow(rTarget);
		qTarget = game_object_script{};
		rTarget = game_object_script{};

		// Allows casting a spell for this update
		hasCasted = false;

		// Get ready spells
		isQReady = myhero->get_spell_state(spellslot::q) == spell_state::Ready;
		isWReady = myhero->get_spell_state(spellslot::w) == spell_state::Ready;
		isEReady = myhero->get_spell_state(spellslot::e) == spell_state::Ready;
		isRReady = myhero->get_spell_state(spellslot::r) == spell_state::Ready;

		// Get buffs
		ultBuff = myhero->get_buff(buff_hash("xerathrshots"));
		qBuff = myhero->get_buff(buff_hash("XerathArcanopulseChargeUp"));
		elderBuff = myhero->get_buff(buff_hash("ElderDragonBuff"));

		// Disable Orb in ult
		orbwalker->set_movement(!ultBuff);
		orbwalker->set_attack(!ultBuff);

		// Get pred & damage of spells && a bunch of useful stuff on every enemies so you don't need to do it multiple times per update
		for (const auto& target : entitylist->get_enemy_heroes())
		{
			if (!target->is_valid() || target->is_dead()) continue;

			qPredictionList[target->get_handle()] = isQReady ? getQShortPred(target) : prediction_output{};
			wPredictionList[target->get_handle()] = isWReady ? getWPred(target) : prediction_output{};
			w2PredictionList[target->get_handle()] = isWReady ? getW2Pred(target) : prediction_output{};
			ePredictionList[target->get_handle()] = isEReady ? getEPred(target) : prediction_output{};
			if (ultBuff)
				rPredictionList[target->get_handle()] = isRReady ? getRPred(target) : prediction_output{};
			qDummyPredictionList[target->get_handle()] = isQReady ? getQDummyPred(target) : prediction_output{};
			if (qBuff)
				q2PredictionList[target->get_handle()] = isQReady ? getQ2Pred(target) : prediction_output{};

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
	}

	bool debuffCantCast()
	{
		// Check if player has any debuff that prevents spell casting
		const auto& stunBuffList = { buff_type::Stun, buff_type::Silence, buff_type::Taunt, buff_type::Polymorph, buff_type::Fear, buff_type::Charm, buff_type::Suppression, buff_type::Knockup, buff_type::Knockback, buff_type::Asleep };
		for (auto&& buff : myhero->get_bufflist())
		{
			if (buff == nullptr || !buff->is_valid() || !buff->is_alive()) continue;
			for (const auto& buffType : stunBuffList)
			{
				if (buff->get_type() == buffType) return true;
			}
		}
		return false;
	}

	bool isCastingSpell()
	{
		// Check if we're already casting a spell
		const auto& castingTime = myhero->get_active_spell() ? myhero->get_active_spell()->cast_start_time() - gametime->get_time() : 0;
		if (myhero->get_active_spell() && !myhero->get_active_spell()->is_auto_attack())
		{
			if (castingTime < 0.033 + getPing() &&  castingTime > 0)
			{
				return !myhero->get_active_spell()->is_channeling() && !myhero->get_active_spell()->get_spell_data()->is_insta();
			}
			else
			{
				lastCast = 0;
			}
		}
		return false;
	}

	bool canCastSpells()
	{
		if (myhero->is_dead()) return false;

		if (myhero->is_recalling()) return false;

		if (!isQReady && !isWReady && !isEReady && !isRReady && !ultBuff) return false;

		if (debuffCantCast()) return false;

		if (lastCast > gametime->get_time() && !ultBuff) return false;

		if (isCastingSpell()) return false;

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
		for (int i = 0; i < size; i++)
		{
			const auto& tsTarget = target_selector->get_target(targets, damage_type::magical);
			if (tsTarget)
			{
				dummyList.push_back(tsTarget);
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
		if (settings::automatic::avoidShields->get_bool())
		{
			std::sort(dummyList.rbegin(), dummyList.rend(), [](game_object_script a, game_object_script b) {
				const float shieldValueA = (a->get_magical_shield() + a->get_all_shield()) * 2;
				if (shieldValueA == 0)
					return false;
				float damageA = 0;
				bool firstDamage = false;
				int shots = 0;
				const auto& accountQ = isQReady || (myhero->get_active_spell() && myhero->get_active_spell()->get_spell_data()->get_name_hash() == 2320506602);
				const auto& accountW = isWReady || timeBeforeWHits(a) < FLT_MAX;
				const auto& accountE = isEReady || willGetHitByE(a);
				const auto& accountR = ultBuff || willGetHitByR(a);
				if (accountQ)
					shots++;
				if (accountW)
					shots++;
				if (accountE)
					shots++;
				if (accountR)
					shots++;
				if (accountQ)
				{
					damageA += getQDamageAlternative(a, --shots, getTotalHP(a) - damageA, firstDamage);
					firstDamage = false;
				}
				if (accountW)
				{
					damageA += getW2DamageAlternative(a, --shots, getTotalHP(a) - damageA, firstDamage);
					firstDamage = false;
				}
				if (accountE)
				{
					damageA += getEDamageAlternative(a, --shots, getTotalHP(a) - damageA, firstDamage);
					firstDamage = false;
				}
				if (accountR)
				{
					damageA += getRDamage(a, --shots, getTotalHP(a) - damageA, firstDamage);
				}
				const float resistanceA = damagelib->calculate_damage_on_unit(myhero, a, damage_type::magical, 1);
				const float resistanceB = damagelib->calculate_damage_on_unit(myhero, b, damage_type::magical, 1);
				const float shieldValueB = (b->get_magical_shield() + b->get_all_shield()) * 2;
				const float effectiveShieldA = shieldValueA / resistanceA;
				const float effectiveShieldB = shieldValueB / resistanceB;
				return damageA < shieldValueA && effectiveShieldA > effectiveShieldB;
				}
			);
		}
		std::sort(dummyList.begin(), dummyList.end(), [](game_object_script a, game_object_script b) {
			return target_selector->get_selected_target() && target_selector->get_selected_target()->get_handle() == a->get_handle();
			}
		);
		targets = dummyList;
	}

	void manualHandling()
	{
		if (!settings::automatic::manualEKey->get_bool() && !settings::automatic::manualRKey->get_bool() && !ultBuff) return;

		for (const auto& target : targets)
		{
			// Get stasis time
			const auto& stasisDuration = stasisInfo[target->get_handle()].stasisTime;

			// Valid target check
			const bool& isValidTarget = target && (customIsValid(target) || stasisDuration > 0) && !target->is_zombie();

			// If not valid then go to next target
			if (!isValidTarget) continue;

			// Enable ult if manual R key is pressed
			if (!ultBuff && isRReady && settings::automatic::manualRKey->get_bool() && settings::automatic::manualREnable->get_bool()) r->cast(hud->get_hud_input_logic()->get_game_cursor_position());

			// Store useful info
			auto ccTime = stunTime[target->get_handle()];
			auto ep = ePredictionList[target->get_handle()];
			auto eLandingTime = getTimeToHit(ep.input, ep, true);
			auto trueELandingTime = getTimeToHit(ep.input, ep, false);

			// Storing useful info
			const auto& canUseE = settings::automatic::manualEKey->get_bool() && couldDamageLater(target, trueELandingTime - 0.5, eDamageList[target->get_handle()]) && stasisDuration <= 0 && isEReady && ePredictionList[target->get_handle()].hitchance > hit_chance::impossible;
			const auto& canUseR = couldDamageLater(target, r->get_delay() + 0.8, getRDamage(target, 0, getTotalHP(target), true)) && (stasisDuration - getPing()) < 2 - r->get_delay() && ultBuff;
			const auto& rCombo = settings::combo::rCombo->get_bool() && orbwalker->combo_mode();

			// If can't do anything on target, go next target
			if (!canUseE && !canUseR) continue;

			// Cast E (chain CC logic)
			if (canUseE && (ccTime - 0.3) < eLandingTime && castE(target, "manual")) return;

			// Cast R
			const auto& rRange = settings::automatic::rRange->get_int() > 0 ? settings::automatic::rRange->get_int() : FLT_MAX;
			if (canUseR)
			{
				if (hud->get_hud_input_logic()->get_game_cursor_position().distance(target->get_position()) <= rRange)
				{
					rTarget = target;
					const auto& manualKey = settings::automatic::manualRKey->get_bool();
					const auto& dashingCast = settings::ultimate::rDash->get_bool() && target->is_dashing();
					const auto& castingSpell = settings::ultimate::rCast->get_bool() && target->get_active_spell() && target->get_active_spell()->cast_start_time() - 0.033 >= gametime->get_time();
					const auto& castingCast = castingSpell && !target->get_active_spell()->get_spell_data()->is_insta() && !target->get_active_spell()->get_spell_data()->mCanMoveWhileChanneling() && !ignoreSpells.contains(target->get_active_spell()->get_spell_data()->get_name_hash());
					const auto& castingImmobile = settings::ultimate::rImmobile->get_bool() && rPredictionList[target->get_handle()].hitchance >= hit_chance::dashing;
					const auto& castingCantDodge = settings::ultimate::rCantDodge->get_bool() && !target->is_dashing() && rPredictionList[target->get_handle()].hitchance >= hit_chance::very_high && target->get_move_speed() * (r->get_delay() + getPing() + 0.066) < r->get_radius();
					const auto& castingStasis = settings::ultimate::rStasis->get_bool() && stasisDuration > 0 && (stasisDuration - getPing() + 0.2) < r->get_delay();
					if (isRReady && (manualKey || dashingCast || castingCast || castingImmobile || castingCantDodge || castingStasis || rCombo) && rPredictionList[target->get_handle()].hitchance > hit_chance::impossible && (stasisDuration - getPing() + 0.2) < r->get_delay())
						castR(target, "manual");
					hasCasted = true;
					return;
				}
			}
		}
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
			auto& ep = ePredictionList[target->get_handle()];
			const auto& eLandingTime = getTimeToHit(ep.input, ep, true);
			const auto& trueELandingTime = getTimeToHit(ep.input, ep, false);

			// Check if X spells can be used on that target
			const auto& canUseQ = settings::combo::qCombo->get_bool() && couldDamageLater(target, q->get_delay() - 0.5, qDamageList[target->get_handle()]) && isQReady && qPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range && myhero->get_distance(target) <= (XERATH_MIN_Q_RANGE + target->get_bounding_radius()) && !qBuff;
			const auto& canUseW = settings::combo::wCombo->get_bool() && couldDamageLater(target, w->get_delay() - 0.5, wDamageList[target->get_handle()]) && isWReady && wPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canUseE = settings::combo::eCombo->get_bool() && couldDamageLater(target, trueELandingTime - 0.5, eDamageList[target->get_handle()]) && isEReady && ePredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canChargeQ = !canUseQ && qPredictionList[target->get_handle()].hitchance < hit_chance::impossible && settings::combo::qCombo->get_bool() && couldDamageLater(target, qCharge->get_delay() + 2.f, qDamageList[target->get_handle()]) && isQReady;
			const auto& canReleaseQ = settings::combo::qCombo->get_bool() && couldDamageLater(target, q2->get_delay() - 0.5, qDamageList[target->get_handle()]) && isQReady && qBuff;

			// If no spells can be used on that target then go to next target
			if (!canUseQ && !canUseW && !canUseE && !canChargeQ && !canReleaseQ) continue;

			auto shouldBreak = false;

			// Charged Q recast
			if (canReleaseQ)
			{
				if (q2PredictionList[target->get_handle()].hitchance > hit_chance::impossible && isValidQ(target))
				{
					if (castQLong(target, "combo")) return;
				}
				if (prediction->get_prediction(target, q2->get_delay() + getPing()).get_unit_position().distance(myhero->get_position()) <= XERATH_MAX_Q_RANGE + target->get_bounding_radius())
				{
					qTarget = target;
					hasCasted = true;
					break;
				}
				else
					continue;
			}

			// E cast
			if (canUseE)
			{
				if ((ccTime - 0.3) < eLandingTime && castE(target, "combo")) return;
				if (ePredictionList[target->get_handle()].hitchance > hit_chance::low && couldDamageLater(target, ccTime + eLandingTime, eDamageList[target->get_handle()]) && ccTime > 0 && ccTime < 2) return;
			};

			// W cast
			if (canUseW)
			{
				const auto& wCenter = settings::combo::wComboCenter->get_bool() && w2PredictionList[target->get_handle()].hitchance >= wPredictionList[target->get_handle()].hitchance;
				shouldBreak = true;
				if (castW(target, "combo", wCenter)) return;
			}

			// Q cast
			if (canUseQ && castQShort(target, "combo")) return;

			// Q charge
			if (canChargeQ && castQCharge(target, "combo")) return;

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
			const bool& isValidTarget = target && customIsValid(target) && !target->is_zombie();

			// If not valid then go to next target
			if (!isValidTarget) continue;

			// Check if X spells can be used on that target
			const auto& canUseQ = settings::harass::qHarass->get_bool() && isQReady && qPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range && myhero->get_distance(target) <= (XERATH_MIN_Q_RANGE + target->get_bounding_radius()) && !qBuff;
			const auto& canUseW = settings::harass::wHarass->get_bool() && isWReady && wPredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canUseE = settings::harass::eHarass->get_bool() && isEReady && ePredictionList[target->get_handle()].hitchance > hit_chance::out_of_range;
			const auto& canChargeQ = !canUseQ && settings::harass::qHarass->get_bool() && isQReady;
			const auto& canReleaseQ = settings::harass::qHarass->get_bool() && isQReady && qBuff;

			// If no spells can be used on that target then go to next target
			if (!canUseQ && !canUseW && !canUseE && !canChargeQ && !canReleaseQ) continue;

			// Store useful info to use in logic
			const auto& ccTime = stunTime[target->get_handle()];
			auto& ep = ePredictionList[target->get_handle()];
			const auto& eLandingTime = getTimeToHit(ep.input, ep, true);
			auto shouldBreak = false;

			// Charged Q recast
			if (canReleaseQ)
			{
				if (q2PredictionList[target->get_handle()].hitchance > hit_chance::impossible && isValidQ(target))
				{
					if (castQLong(target, "harass")) return;
				}
				if (prediction->get_prediction(target, q2->get_delay() + getPing()).get_unit_position().distance(myhero->get_position()) <= (XERATH_MAX_Q_RANGE + target->get_bounding_radius()))
				{
					qTarget = target;
					hasCasted = true;
					break;
				}
				else
					continue;
			}

			// E cast
			if (canUseE)
			{
				if ((ccTime - 0.3) < eLandingTime && castE(target, "harass")) return;
				if (ePredictionList[target->get_handle()].hitchance > hit_chance::low && couldDamageLater(target, ccTime + eLandingTime, eDamageList[target->get_handle()]) && ccTime > 0 && ccTime < 2) return;
			};

			// W cast
			if (canUseW && couldDamageLater(target, w->get_delay() + 0.5, wDamageList[target->get_handle()]))
			{
				const auto& wCenter = settings::harass::wHarassCenter->get_bool() && w2PredictionList[target->get_handle()].hitchance >= wPredictionList[target->get_handle()].hitchance;
				shouldBreak = true;
				if (castW(target, "harass", wCenter)) return;
			}

			// Q cast
			if (canUseQ && castQShort(target, "harass")) return;

			// Q charge
			if (canChargeQ && castQCharge(target, "harass")) return;

			if (shouldBreak) break;

		}
	}

	void particleHandling()
	{
		// Store particle settings
		auto particleE = settings::automatic::eParticle->get_bool() && isEReady;
		auto particleW = settings::automatic::wParticle->get_bool() && isWReady;
		auto particleQ = settings::automatic::qParticle->get_bool() && isQReady && qBuff;

		// Checking if particles are valid, if they're not, delete them from the list
		particlePredList.erase(std::remove_if(particlePredList.begin(), particlePredList.end(), [](const particleStruct& x)
			{
				return !x.obj->is_valid() || x.owner->is_dead() || x.time + x.castTime <= gametime->get_time();
			}
		),
			particlePredList.end());

		if (hasCasted || (!particleE && !particleW && !particleQ)) return;

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
					obj.castingPos = obj.obj->get_position();
				}
			}
			else if (obj.isZed)
			{
				obj.castingPos = obj.target->get_position() + (obj.owner->get_direction() * (obj.target->get_bounding_radius() + obj.owner->get_bounding_radius()));
			}

			// Check if cast position isn't too far enough
			if ((myhero->get_position().distance(obj.castingPos) - obj.owner->get_bounding_radius()) > std::max(XERATH_MAX_Q_RANGE, ePredictionList[obj.owner->get_handle()].input.range)) continue;

			// Gathering enough data to cast on particles
			const auto& distance = myhero->get_position().distance(obj.castingPos) - (obj.owner->get_bounding_radius());
			const auto& eLandingTime = std::max(e->get_delay(), (distance / e->get_speed()) + e->get_delay());
			const auto& particleTime = (obj.time + obj.castTime) - gametime->get_time();
			const auto& eCanDodge = obj.owner->get_move_speed() * ((eLandingTime - particleTime) + getPing()) > e->get_radius() + obj.owner->get_bounding_radius();
			const auto& wCanDodge = obj.owner->get_move_speed() * ((w->get_delay() - particleTime) + getPing()) > w->get_radius();
			const auto& qCanDodge = obj.owner->get_move_speed() * ((q->get_delay() - particleTime) + getPing()) > q->get_radius();
			const auto& collisionList = e->get_collision(myhero->get_position(), { obj.castingPos });
			const auto& canE = particleE && !eCanDodge && myhero->get_position().distance(obj.castingPos) <= ePredictionList[obj.owner->get_handle()].input.range && collisionList.empty();
			const auto& canW = particleW && !wCanDodge && myhero->get_position().distance(obj.castingPos) <= w->range();
			const auto& canQ = particleQ && !qCanDodge && myhero->get_position().distance(obj.castingPos) <= charged_range(XERATH_MAX_Q_RANGE, XERATH_MIN_Q_RANGE, 1.5) + obj.owner->get_bounding_radius() - 50;

			// Try to cast E if possible
			if (canE && (particleTime - getPing() + 0.05 <= eLandingTime))
			{
				e->cast(obj.castingPos);
				hasCasted = true;
				return;
			}
			// Try to cast W if possible
			else if (canW && !canE && (particleTime - getPing() + 0.2) <= w->get_delay())
			{
				w->cast(obj.castingPos);
				hasCasted = true;
				return;
			}
			// Try to cast Q if possible
			else if (canQ && (particleTime - getPing() + 0.2) <= q2->get_delay())
			{
				myhero->update_charged_spell(q2->get_slot(), obj.castingPos, true);
				hasCasted = true;
				return;
			}
		}
	}

	void automatic()
	{
		// Check if you didn't already cast
		if (hasCasted) return;

		// Store every settings
		auto ccE = settings::automatic::eStun->get_bool() && isEReady;
		auto ccW = settings::automatic::wStun->get_bool() && isWReady;
		auto dashE = settings::automatic::eDash->get_bool() && isEReady;
		auto dashW = settings::automatic::wDash->get_bool() && isWReady;
		auto dashQ = settings::automatic::qDash->get_bool() && isQReady && qBuff;
		auto castingE = settings::automatic::eCast->get_bool() && isEReady;
		auto castingW = settings::automatic::wCast->get_bool() && isWReady;
		auto castingQ = settings::automatic::qCast->get_bool() && isQReady && qBuff;
		auto channelE = settings::automatic::eChannel->get_bool() && isEReady;
		auto channelW = settings::automatic::wChannel->get_bool() && isWReady;
		auto stasisE = settings::automatic::eStasis->get_bool() && isEReady;
		auto stasisW = settings::automatic::wStasis->get_bool() && isWReady;
		auto stasisQ = settings::automatic::qStasis->get_bool() && isQReady && qBuff;

		// Stop if player doesn't want to use any auto stuff
		if (!ccE && !ccW && !dashE && !dashW && !dashQ && !castingE && !castingW && !castingQ && !channelE && !channelW && !stasisE && !stasisW  && !stasisQ && particlePredList.empty()) return;

		// Loop through every sorted targets
		for (const auto& target : targets)
		{
			const auto& stasisDuration = stasisInfo[target->get_handle()].stasisTime;
			// Valid target check
			const bool& isValidTarget = target && (customIsValid(target) || stasisDuration > 0) && !target->is_zombie();
			// If not valid then go to next target
			if (!isValidTarget) continue;

			const auto& dashing = target->is_dashing() || qPredictionList[target->get_handle()].hitchance == hit_chance::dashing || wPredictionList[target->get_handle()].hitchance == hit_chance::dashing || w2PredictionList[target->get_handle()].hitchance == hit_chance::dashing || ePredictionList[target->get_handle()].hitchance == hit_chance::dashing;
			const auto& ccTime = stunTime[target->get_handle()];
			const auto& channelingSpell = target->is_casting_interruptible_spell() >= 1 || isRecalling(target);
			const auto& ccCast = ccTime > 0 && (ccE || ccW);
			const auto& dashingCast = dashing && (dashE || dashW || dashQ);
			const auto& castingSpell = target->get_active_spell() && target->get_active_spell()->cast_start_time() - 0.033 >= gametime->get_time();
			const auto& castingCast = castingSpell && !target->get_active_spell()->get_spell_data()->is_insta() && !target->get_active_spell()->get_spell_data()->mCanMoveWhileChanneling() && (castingE || castingW || castingQ) && !ignoreSpells.contains(target->get_active_spell()->get_spell_data()->get_name_hash());
			const auto& channelingCast = channelingSpell && (channelE || channelW);
			const auto& stasisCast = stasisDuration > 0 && (stasisE || stasisW || stasisQ);
			if (!ccCast && !dashingCast && !castingCast && !channelingCast && !stasisCast) continue;

			auto& ep = ePredictionList[target->get_handle()];
			auto eLandingTime = getTimeToHit(ep.input, ep, true);
			const auto& wCenter = w2PredictionList[target->get_handle()].hitchance >= wPredictionList[target->get_handle()].hitchance;

			// Cast on stasis targets
			if (stasisCast)
			{
				// Cast E on stasis
				if (stasisE && (stasisDuration + 0.05) < eLandingTime && castE(target, "stasis")) break;
				// Cast W on stasis
				if (stasisW && (stasisDuration + 0.2 - getPing()) < w->get_delay() && castW(target, "stasis", wCenter)) break;
				// Cast Q on stasis
				if (stasisQ && (stasisDuration + 0.2 - getPing()) < q2->get_delay() && castQLong(target, "stasis")) break;
			}

			// Next part shouldn't cast on stasis targets
			if (stasisDuration > 0) continue;

			// Cast on stun logic
			if (ccCast) {
				// Cast E on stun with chain CC logic
				if (ccE && (ccTime - 0.3) < eLandingTime && castE(target, "stun")) break;
				// Cast W on stun with chain CC logic
				if (ccW && (ccTime - 0.3 - getPing()) < w->get_delay() && castW(target, "stun", wCenter)) break;
			}

			// Cast on dash logic
			if (dashingCast) {
				// Cast E on dash
				if (dashE && castE(target, "dash")) break;
				// Cast W on dash
				if (dashW && castW(target, "dash", wCenter)) break;
				// Cast Q on dash
				if (dashQ && castQLong(target, "dash")) break;
			}

			// Cast on casting logic
			if (castingCast) {
				// Cast E on casting
				if (castingE && castE(target, "casting")) break;
				// Cast W on casting
				if (castingW && castW(target, "casting", wCenter)) break;
				// Cast Q on casting
				if (castingQ && castQLong(target, "casting")) break;
			}

			// Cast on channeling logic
			if (channelingCast) {
				// Cast E on channel
				if (channelE && castE(target, "channeling")) break;
				// Cast W on channel
				if (channelW && castW(target, "channeling", wCenter)) break;
			}

		}

		// Particle pred handling
		if (!particlePredList.empty())
			particleHandling();

	}

	void glowManager()
	{
		if (settings::draws::qIndicator->get_bool() && qTarget && qTarget->is_valid() && !qTarget->is_dead())
		{
			glow->apply_glow(qTarget, MAKE_COLOR(255, 127, 0, 255), 3, 0);
		}
		if (settings::draws::rIndicator->get_bool() && rTarget && rTarget->is_valid() && !rTarget->is_dead())
		{
			glow->apply_glow(rTarget, MAKE_COLOR(255, 0, 0, 255), 3, 0);
		}
	}

	void createMenu()
	{
		// Main tab
		mainMenu = menu->create_tab("open.xerath", "OpenXerath");
		mainMenu->set_texture(myhero->get_square_icon_portrait());

		// Combo settings
		const auto comboTab = mainMenu->add_tab("open.xerath.combo", "Combo");
		settings::combo::qCombo = comboTab->add_checkbox("open.xerath.combo.qcombo", "Q combo", true);
		settings::combo::qCombo->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
		settings::combo::wCombo = comboTab->add_checkbox("open.xerath.combo.wcombo", "W combo", true);
		settings::combo::wCombo->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
		settings::combo::wComboCenter = comboTab->add_checkbox("open.xerath.combo.wComboCenter", "^ Try to hit center?", false);
		settings::combo::eCombo = comboTab->add_checkbox("open.xerath.combo.ecombo", "E combo", true);
		settings::combo::eCombo->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
		settings::combo::rCombo = comboTab->add_checkbox("open.xerath.combo.rcombo", "R2 combo", true);
		settings::combo::rCombo->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

		// Harass settings
		const auto harassTab = mainMenu->add_tab("open.xerath.harass", "Harass");
		settings::harass::qHarass = harassTab->add_checkbox("open.xerath.combo.qharass", "Q harass", true);
		settings::harass::qHarass->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
		settings::harass::wHarass = harassTab->add_checkbox("open.xerath.combo.wharass", "W harass", true);
		settings::harass::wHarass->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
		settings::harass::wHarassCenter = harassTab->add_checkbox("open.xerath.harass.wComboCenter", "^ Try to hit center?", false);
		settings::harass::eHarass = harassTab->add_checkbox("open.xerath.combo.eharass", "E harass", true);
		settings::harass::eHarass->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());

		// Draw settings
		const auto drawTab = mainMenu->add_tab("open.xerath.draws", "Drawings");

		// Draw spellrange tab
		const auto drawRangeTab = drawTab->add_tab("open.xerath.draws.ranges", "Spell ranges");
		settings::draws::spellRanges::qRange = drawRangeTab->add_checkbox("open.xerath.draws.ranges.qrange", "Draw Q range", true);
		settings::draws::spellRanges::qRange->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
		settings::draws::spellRanges::wRange = drawRangeTab->add_checkbox("open.xerath.draws.ranges.wrange", "Draw W range", true);
		settings::draws::spellRanges::wRange->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
		settings::draws::spellRanges::eRange = drawRangeTab->add_checkbox("open.xerath.draws.ranges.erange", "Draw E range", true);
		settings::draws::spellRanges::eRange->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
		settings::draws::spellRanges::rRange = drawRangeTab->add_checkbox("open.xerath.draws.ranges.rrange", "Draw R range", true);
		settings::draws::spellRanges::rRange->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
		settings::draws::spellRanges::rMinimapRange = drawRangeTab->add_checkbox("open.xerath.draws.ranges.rminimaprange", "Draw R range on minimap", true);
		settings::draws::spellRanges::rNearMouseRange = drawRangeTab->add_checkbox("open.xerath.draws.ranges.rnearmouserange", "Draw near mouse R range", true);

		// Normal draws
		settings::draws::wRadius = drawTab->add_checkbox("open.xerath.draws.wradius", "Draw W on ground", true);
		settings::draws::rRadius = drawTab->add_checkbox("open.xerath.draws.rradius", "Draw R on ground", true);
		settings::draws::rDamage = drawTab->add_checkbox("open.xerath.draws.rdamage", "Draw R damage", true);
		settings::draws::rDamageText = drawTab->add_checkbox("open.xerath.draws.rdamagetext", "Draw R damage text", true);
		settings::draws::rKillList = drawTab->add_checkbox("open.xerath.draws.rkilllist", "Show R kill list", true);
		settings::draws::rKillListRangeIgnore = drawTab->add_checkbox("open.xerath.draws.rkilllistrangeignore", "R kill list range ignore", false);
		settings::draws::killListXOffset = drawTab->add_slider("open.xerath.draws.xoffset", "Horizontal position", 0, -2000, 2000);
		settings::draws::killListYOffset = drawTab->add_slider("open.xerath.draws.yoffset", "Vertical position", 0, -2000, 2000);
		settings::draws::rIndicator = drawTab->add_checkbox("open.xerath.draws.rindicator", "Show R target", true);
		settings::draws::qIndicator = drawTab->add_checkbox("open.xerath.draws.qindicator", "Show Q target", true);
		settings::draws::particlePos = drawTab->add_checkbox("open.xerath.draws.particlepos", "Draw particle pred positions", false);
		settings::draws::particlePos->set_tooltip("Keep it disabled if you use OpenUtilities teleport tracker");
		settings::draws::stasisPos = drawTab->add_checkbox("open.xerath.draws.stasispos", "Draw stasis pred positions", true);

		// Hitchance tab
		const auto hitchanceTab = mainMenu->add_tab("open.xerath.hitchance", "Hitchance");
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
		settings::hitchance::qHitchance = hitchanceTab->add_combobox("open.xerath.hitchance.qhitchance", "Q Hitchance", combo_elements, 2);
		settings::hitchance::wHitchance = hitchanceTab->add_combobox("open.xerath.hitchance.whitchance", "W Hitchance", combo_elements, 2);
		settings::hitchance::eHitchance = hitchanceTab->add_combobox("open.xerath.hitchance.ehitchance", "E Hitchance", combo_elements, 2);
		settings::hitchance::rHitchance = hitchanceTab->add_combobox("open.xerath.hitchance.rhitchance", "R Hitchance", combo_elements, 2);

		// Misc tab
		const auto miscTab = mainMenu->add_tab("open.xerath.misc", "Misc");
		settings::automatic::manualEKey = miscTab->add_hotkey("open.xerath.misc.manuale", "Manual E key", TreeHotkeyMode::Hold, 0x4A, false);
		settings::automatic::manualRKey = miscTab->add_hotkey("open.xerath.misc.manualr", "Manual R key", TreeHotkeyMode::Hold, 0x54, false);
		settings::automatic::rRange = miscTab->add_slider("open.xerath.misc.manualrrange", "Ult near mouse range", 0, 0, 1500);
		settings::automatic::rRange->set_tooltip("Set to 0 to disable near mouse feature");
		settings::automatic::manualREnable = miscTab->add_checkbox("open.xerath.misc.manualrenable", "Start channeling R with manual R key", false);
		settings::automatic::avoidShields = miscTab->add_checkbox("open.xerath.misc.avoidshields", "Try to avoid shields", true);
		settings::automatic::eStun = miscTab->add_checkbox("open.xerath.misc.estun", "Auto E on stun", true);
		settings::automatic::wStun = miscTab->add_checkbox("open.xerath.misc.wstun", "Auto W on stun", true);
		settings::automatic::eDash = miscTab->add_checkbox("open.xerath.misc.edash", "Auto E on dash", true);
		settings::automatic::wDash = miscTab->add_checkbox("open.xerath.misc.wdash", "Auto W on dash", true);
		settings::automatic::qDash = miscTab->add_checkbox("open.xerath.misc.qdash", "Auto Q recast on dash", true);
		settings::automatic::eCast = miscTab->add_checkbox("open.xerath.misc.ecast", "Auto E on cast", true);
		settings::automatic::wCast = miscTab->add_checkbox("open.xerath.misc.wcast", "Auto W on cast", true);
		settings::automatic::qCast = miscTab->add_checkbox("open.xerath.misc.qcast", "Auto Q recast on cast", false);
		settings::automatic::eChannel = miscTab->add_checkbox("open.xerath.misc.echannel", "Auto E on channel", true);
		settings::automatic::wChannel = miscTab->add_checkbox("open.xerath.misc.wchannel", "Auto W on channel", true);
		settings::automatic::eStasis = miscTab->add_checkbox("open.xerath.misc.estasis", "Auto E on stasis", true);
		settings::automatic::wStasis = miscTab->add_checkbox("open.xerath.misc.wstasis", "Auto W on stasis", true);
		settings::automatic::qStasis = miscTab->add_checkbox("open.xerath.misc.qstasis", "Auto Q recast on stasis", true);
		settings::automatic::eParticle = miscTab->add_checkbox("open.xerath.misc.eparticle", "Auto E on particle", true);
		settings::automatic::wParticle = miscTab->add_checkbox("open.xerath.misc.wparticle", "Auto W on particle", true);
		settings::automatic::qParticle = miscTab->add_checkbox("open.xerath.misc.qparticle", "Auto Q recast on particle", true);

		// Ult tab
		const auto ultTab = mainMenu->add_tab("open.xerath.ultimate", "Ultimate");
		settings::ultimate::rCast = ultTab->add_checkbox("open.xerath.ultimate.rcast", "Cast R2 on cast", true);
		settings::ultimate::rDash = ultTab->add_checkbox("open.xerath.ultimate.rdash", "Cast R2 on dash", true);
		settings::ultimate::rImmobile = ultTab->add_checkbox("open.xerath.ultimate.rimmobile", "Cast R2 on immobile", true);
		settings::ultimate::rCantDodge = ultTab->add_checkbox("open.xerath.ultimate.rcantdodge", "Cast R2 on undodgeable", true);
		settings::ultimate::rStasis = ultTab->add_checkbox("open.xerath.ultimate.rstasis", "Cast R2 on stasis", true);

		// Misc
		settings::lowSpec = mainMenu->add_checkbox("open.xerath.lowspec", "Low spec mode (tick limiter)", false);
	}

	void on_update()
	{
		// Limit ticks (for low spec mode)
		if (settings::lowSpec->get_bool() && limitedTick(SERVER_TICKRATE)) return;

		// Sort targets
		targetSelectorSort();

		// Pred, damage && other calcs needed for many things
		calcs();

		// Check if player can cast spells
		if (!canCastSpells()) return;

		// Manual casts
		manualHandling();

		// Combo mode
		combo();

		// Harass mode
		harass();

		// Auto cast
		automatic();

		// Glow manager
		glowManager();

	}

	void on_draw()
	{
		// Spellranges

		// Q
		if (settings::draws::spellRanges::qRange->get_bool())
		{
			auto alpha = isQReady ? 255 : 50;
			draw_manager->add_circle(myhero->get_position(), true_charged_range(1500, 750, 1.5), MAKE_COLOR(204, 127, 0, alpha), 2);
			draw_manager->add_circle(myhero->get_position(), 1500, MAKE_COLOR(204, 127, 0, alpha), 2);
		}

		// W
		if (settings::draws::spellRanges::wRange->get_bool())
		{
			auto alpha = isWReady ? 255 : 50;
			draw_manager->add_circle(myhero->get_position(), XERATH_W_RANGE, MAKE_COLOR(0, 255, 255, alpha), 2);
		}

		// E
		if (settings::draws::spellRanges::eRange->get_bool())
		{
			auto alpha = isEReady ? 255 : 50;
			draw_manager->add_circle(myhero->get_position(), XERATH_E_RANGE, MAKE_COLOR(0, 127, 255, alpha), 2);
		}

		// R
		if (settings::draws::spellRanges::rRange->get_bool())
		{
			auto alpha = isRReady ? 255 : 50;
			draw_manager->add_circle(myhero->get_position(), XERATH_R_RANGE, MAKE_COLOR(255, 127, 0, alpha), 2);
		}

		// Near mouse R range
		if (settings::draws::spellRanges::rNearMouseRange->get_bool() && settings::automatic::rRange->get_int() > 0 && ultBuff)
		{
			draw_manager->add_circle(hud->get_hud_input_logic()->get_game_cursor_position(), settings::automatic::rRange->get_int(), MAKE_COLOR(255, 127, 0, 255), 2);
		}

		// Misc

		// Draw W on ground
		if (settings::draws::wRadius->get_bool()) {
			for (const auto& particle : particleList) {
				draw_manager->add_filled_circle(particle.particle->get_position(), XERATH_W_OUTER_RADIUS * std::min(1.f, (1 / (XERATH_W_PARTICLE_TIME / (gametime->get_time() - particle.creationTime)))), MAKE_COLOR(0, 255, 255, 64));
			}
		}

		// Draw R on ground
		if (settings::draws::rRadius->get_bool())
		{
			for (const auto& particle : ultParticleList) {
				draw_manager->add_filled_circle(particle.particle->get_position(), r->get_radius() * std::min(1.f, (1 / (XERATH_R_PARTICLE_TIME / (gametime->get_time() - particle.creationTime)))), MAKE_COLOR(255, 127, 0, 64));
			}
		}

		// Draw misc
		int index = 0;
		for (const auto& target : entitylist->get_enemy_heroes())
		{

			// Draw stasis pred pos
			auto stasisData = stasisInfo[target->get_handle()];
			if (settings::draws::stasisPos->get_bool() && stasisData.stasisTime > 0 && stasisData.stasisEnd < gametime->get_time())
			{
				auto castTime = stasisData.stasisEnd - stasisData.stasisStart;
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

		// Target indicators management
		if (settings::draws::qIndicator->get_bool() && qTarget && qTarget->is_valid() && !qTarget->is_dead())
		{
			draw_manager->add_filled_circle(qTarget->get_position(), fmod((250.f * -gametime->get_time()), (50 + qTarget->get_bounding_radius())), MAKE_COLOR(255, 127, 0, 64));
		}
		if (settings::draws::rIndicator->get_bool() && rTarget && rTarget->is_valid() && !rTarget->is_dead())
		{
			draw_manager->add_filled_circle(rTarget->get_position(), fmod((250.f * -gametime->get_time()), (50 + rTarget->get_bounding_radius())), MAKE_COLOR(255, 0, 0, 64));
		}
	}

	void on_draw_real()
	{
		// Minimap R range
		if (settings::draws::spellRanges::rMinimapRange->get_bool() && isRReady)
		{
			draw_manager->draw_circle_on_minimap(myhero->get_position(), XERATH_R_RANGE, MAKE_COLOR(255, 127, 0, 255), 2);
		}

		// Draw W on ground
		if (settings::draws::wRadius->get_bool()) {
			for (const auto& particle : particleList) {
				draw_manager->add_circle(particle.particle->get_position(), XERATH_W_OUTER_RADIUS, MAKE_COLOR(0, 0, 255, 255), 2);
				draw_manager->add_circle(particle.particle->get_position(), XERATH_W_OUTER_RADIUS * std::min(1.f, (1 / (XERATH_W_PARTICLE_TIME / (gametime->get_time() - particle.creationTime)))), MAKE_COLOR(0, 255, 255, 255), 2);
			}
		}

		// Draw R on ground
		if (settings::draws::rRadius->get_bool())
		{
			for (const auto& particle : ultParticleList) {
				draw_manager->add_circle(particle.particle->get_position(), r->get_radius(), MAKE_COLOR(255, 0, 0, 255), 2);
				draw_manager->add_circle(particle.particle->get_position(), r->get_radius() * std::min(1.f, (1 / (XERATH_R_PARTICLE_TIME / (gametime->get_time() - particle.creationTime)))), MAKE_COLOR(255, 127, 0, 255), 2);
			}
		}

		// Draw R damage & damage text
		int index = 0;
		for (const auto& target : entitylist->get_enemy_heroes())
		{
			if (!target->is_valid()) continue;

			if (settings::draws::rKillList->get_bool() && customIsValid(target) && rDamageList[target->get_handle()].kills && (settings::draws::rKillListRangeIgnore->get_bool() || target->get_position().distance(myhero->get_position()) <= r->range()))
			{
				const auto& key = index++;
				const auto& position = vector(1350.f + settings::draws::killListXOffset->get_int(), 80.f + settings::draws::killListYOffset->get_int() + (key * 50));
				draw_manager->add_text_on_screen(position, MAKE_COLOR(255, 0, 0, 255), 22, "%s is killable in %i %s", target->get_model_cstr(), rDamageList[target->get_handle()].shots, rDamageList[target->get_handle()].shots > 1 ? "shots" : "shot");
			}

			// Draw stasis pred pos
			auto stasisData = stasisInfo[target->get_handle()];
			if (settings::draws::stasisPos->get_bool() && stasisData.stasisTime > 0 && stasisData.stasisEnd < gametime->get_time())
			{
				draw_manager->add_circle(target->get_position(), target->get_bounding_radius(), MAKE_COLOR(255, 255, 0, 255), 2);
				auto castTime = stasisData.stasisEnd - stasisData.stasisStart;
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
						draw_manager->add_text_on_screen(bar_pos, D3DCOLOR_ARGB(255, 255, 0, 0), 20, "Killable -> %d %s", rDamageList[target->get_handle()].shots, rDamageList[target->get_handle()].shots > 1 ? "shots" : "shot");
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

		// Target indicators management
		if (settings::draws::qIndicator->get_bool() && qTarget && qTarget->is_valid() && !qTarget->is_dead())
		{
			draw_manager->add_circle(qTarget->get_position(), 50 + qTarget->get_bounding_radius(), MAKE_COLOR(255, 127, 0, 255), 2);
			draw_manager->add_circle(qTarget->get_position(), fmod((250.f * -gametime->get_time()), (50 + qTarget->get_bounding_radius())), MAKE_COLOR(255, 127, 0, 255), 2);
		}
		if (settings::draws::rIndicator->get_bool() && rTarget && rTarget->is_valid() && !rTarget->is_dead())
		{
			draw_manager->add_circle(rTarget->get_position(), 50 + rTarget->get_bounding_radius(), MAKE_COLOR(255, 0, 0, 255), 2);
			draw_manager->add_circle(rTarget->get_position(), fmod((250.f * -gametime->get_time()), (50 + rTarget->get_bounding_radius())), MAKE_COLOR(255, 0, 0, 255), 2);
		}
	}

	void on_create(const game_object_script obj)
	{
		// Get object name hash
		const auto& object_hash = spell_hash_real(obj->get_name_cstr());

		// Get emitter hash
		const auto& emitterHash = obj->get_emitter_resources_hash();

		// Get Xerath W and R particles & store them
		switch (emitterHash)
		{
		case buff_hash("Xerath_W_aoe_green"):
		{
			if (obj->get_emitter() && obj->get_emitter()->is_ally())
				particleList.push_back({ .particle = obj, .creationTime = gametime->get_time() });
			return;
		}
		case buff_hash("Xerath_R_aoe_reticle_green"):
		{
			if (obj->get_emitter() && obj->get_emitter()->is_ally())
				ultParticleList.push_back({ .particle = obj, .creationTime = gametime->get_time() });
			return;
		}
		}

		// Manage missiles
		if (obj->is_missile()) {
			switch (obj->get_missile_sdata()->get_name_hash())
			{
			case spell_hash("XerathMageSpearMissile"):
			{
				if (entitylist->get_object(obj->missile_get_sender_id())->is_ally())
				{
					eMissileList.push_back(obj);
				}
			}
			}
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
		switch (obj->get_emitter_resources_hash())
		{
		case buff_hash("Xerath_W_aoe_green"):
		{
			// Delete Xerath W in list when a W particle gets deleted with handle comparison
			if (obj->get_emitter() && obj->get_emitter()->is_ally())
				std::erase_if(particleList, [obj](particleData& particle) {return particle.particle->get_handle() == obj->get_handle(); });
			return;
		}
		case buff_hash("Xerath_R_aoe_reticle_green"):
		{
			// Delete Xerath R in list when a R particle gets deleted with handle comparison
			if (obj->get_emitter() && obj->get_emitter()->is_me())
			{
				std::erase_if(ultParticleList, [obj](particleData& particle) {return particle.particle->get_handle() == obj->get_handle(); });
			}
			return;
		}
		}
		// Manage missiles
		if (obj->is_missile()) {
			switch (obj->get_missile_sdata()->get_name_hash())
			{
			case spell_hash("XerathLocusPulse"):
			{
				if (entitylist->get_object(obj->missile_get_sender_id())->is_me())
				{
					rShots -= 1;
					return;
				}
			}
			case spell_hash("XerathMageSpearMissile"):
			{
				if (entitylist->get_object(obj->missile_get_sender_id())->is_ally())
				{
					std::erase_if(eMissileList, [obj](game_object_script& missile) {return missile->get_handle() == obj->get_handle(); });
				}
			}
			}
		}
	}

	void on_buff(game_object_script& sender, buff_instance_script& buff, const bool gain)
	{
		// Detect if someone is reviving from Guardian Angel
		if (!gain && sender->is_valid() && buff->get_hash_name() == buff_hash("willrevive") && sender->is_playing_animation(buff_hash("Death")) && sender->has_item(ItemId::Guardian_Angel) != spellslot::invalid)
		{
			guardianReviveTime[sender->get_handle()] = gametime->get_time() + 4;
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
		if (sender->get_handle() == myhero->get_handle())
		{
			lastCast = 0;
			if (spell->get_spell_data()->get_name_hash() == spell_hash("XerathLocusOfPower2"))
			{
				rShots = 2 + myhero->get_spell(spellslot::r)->level();
			}
		}
	}

	void load()
	{
		// Spell registering

		// Q
		q = plugin_sdk->register_spell(spellslot::q, XERATH_MIN_Q_RANGE);
		q->set_skillshot(0.55f, 70.f, FLT_MAX, {}, skillshot_type::skillshot_line);
		q->set_spell_lock(false);

		// W
		w = plugin_sdk->register_spell(spellslot::w, XERATH_W_RANGE);
		w->set_skillshot(XERATH_W_PARTICLE_TIME, 275.f, FLT_MAX, {}, skillshot_type::skillshot_circle);
		w->set_spell_lock(false);

		// E
		e = plugin_sdk->register_spell(spellslot::e, XERATH_E_RANGE);
		e->set_skillshot(0.25f, 60.f, 1400.f, { collisionable_objects::minions, collisionable_objects::heroes, collisionable_objects::yasuo_wall }, skillshot_type::skillshot_line);
		e->set_spell_lock(false);

		// R
		r = plugin_sdk->register_spell(spellslot::r, XERATH_R_RANGE);
		r->set_skillshot(XERATH_R_PARTICLE_TIME, 200.f, FLT_MAX, {}, skillshot_type::skillshot_circle);
		r->set_spell_lock(false);

		// Q dummy
		qCharge = plugin_sdk->register_spell(spellslot::q, XERATH_MAX_Q_RANGE);
		qCharge->set_skillshot(0.52f, 70.f, 1250, {}, skillshot_type::skillshot_line);
		qCharge->set_spell_lock(false);

		// Q charged
		q2 = plugin_sdk->register_spell(spellslot::q, XERATH_MIN_Q_RANGE);
		q2->set_skillshot(0.52f, 70.f, FLT_MAX, {}, skillshot_type::skillshot_line);
		q2->set_spell_lock(false);



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
	}

}
