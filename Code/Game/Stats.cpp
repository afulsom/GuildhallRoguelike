#include "Game/Stats.hpp"
#include "Engine/Math/MathUtils.hpp"


Stats::Stats()
	: m_stats()
{

}

Stats Stats::CalculateRandomStatsInRange(const Stats& minStats, const Stats& maxStats)
{
	Stats newStats;

	newStats.m_stats[STAT_STRENGTH] = GetRandomIntInRange(minStats.m_stats[STAT_STRENGTH], maxStats.m_stats[STAT_STRENGTH]);
	newStats.m_stats[STAT_AGILITY] = GetRandomIntInRange(minStats.m_stats[STAT_AGILITY], maxStats.m_stats[STAT_AGILITY]);
	newStats.m_stats[STAT_MAGIC] = GetRandomIntInRange(minStats.m_stats[STAT_MAGIC], maxStats.m_stats[STAT_MAGIC]);
	newStats.m_stats[STAT_ENDURANCE] = GetRandomIntInRange(minStats.m_stats[STAT_ENDURANCE], maxStats.m_stats[STAT_ENDURANCE]);
	newStats.m_stats[STAT_LUCK] = GetRandomIntInRange(minStats.m_stats[STAT_LUCK], maxStats.m_stats[STAT_LUCK]);
	newStats.m_stats[STAT_MAX_HP] = GetRandomIntInRange(minStats.m_stats[STAT_MAX_HP], maxStats.m_stats[STAT_MAX_HP]);

	return newStats;
}
