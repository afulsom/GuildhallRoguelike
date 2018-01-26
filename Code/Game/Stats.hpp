#pragma once


enum StatID
{
	STAT_STRENGTH,
	STAT_AGILITY,
	STAT_MAGIC,
	STAT_ENDURANCE,
	STAT_LUCK,
	STAT_MAX_HP,
	NUM_STATS
};

class Stats
{
public:
	Stats();

	int m_stats[NUM_STATS];

	static Stats CalculateRandomStatsInRange(const Stats& minStats, const Stats& maxStats);

	Stats& operator=(const Stats& statsToAssign);
	Stats operator+(const Stats& statsToAdd);
	Stats& operator+=(const Stats& statsToAdd);
	int& operator [](StatID statIndex);
	int operator [](StatID statIndex) const;
};



inline Stats& Stats::operator=(const Stats& statsToAssign)
{
	m_stats[STAT_STRENGTH] = statsToAssign[STAT_STRENGTH];
	m_stats[STAT_AGILITY] = statsToAssign[STAT_AGILITY];
	m_stats[STAT_MAGIC] = statsToAssign[STAT_MAGIC];
	m_stats[STAT_ENDURANCE] = statsToAssign[STAT_ENDURANCE];
	m_stats[STAT_LUCK] = statsToAssign[STAT_LUCK];
	m_stats[STAT_MAX_HP] = statsToAssign[STAT_MAX_HP];

	return *this;
}

inline Stats Stats::operator+(const Stats& statsToAdd)
{
	Stats outputStats = *this;

	outputStats[STAT_STRENGTH] += statsToAdd[STAT_STRENGTH];
	outputStats[STAT_AGILITY] += statsToAdd[STAT_AGILITY];
	outputStats[STAT_MAGIC] += statsToAdd[STAT_MAGIC];
	outputStats[STAT_ENDURANCE] += statsToAdd[STAT_ENDURANCE];
	outputStats[STAT_LUCK] += statsToAdd[STAT_LUCK];
	outputStats[STAT_MAX_HP] += statsToAdd[STAT_MAX_HP];

	return outputStats;
}

inline Stats& Stats::operator+=(const Stats& statsToAdd)
{
	m_stats[STAT_STRENGTH] += statsToAdd[STAT_STRENGTH];
	m_stats[STAT_AGILITY] += statsToAdd[STAT_AGILITY];
	m_stats[STAT_MAGIC] += statsToAdd[STAT_MAGIC];
	m_stats[STAT_ENDURANCE] += statsToAdd[STAT_ENDURANCE];
	m_stats[STAT_LUCK] += statsToAdd[STAT_LUCK];
	m_stats[STAT_MAX_HP] += statsToAdd[STAT_MAX_HP];

	return *this;
}

inline int& Stats::operator [](StatID statIndex)
{
	return m_stats[statIndex];
}

inline int Stats::operator [](StatID statIndex) const
{
	return m_stats[statIndex];
}
