#ifndef __SECTOR_H__
#define __SECTOR_H__

struct st_Sector
{
	int	sec_y;
	int sec_x;
	bool operator==(const st_Sector& copy) const
	{
		if (sec_y == copy.sec_y && sec_x == copy.sec_x)
			return true;
		return false;
	}
	bool operator!=(const st_Sector& copy) const
	{
		return !(*this == copy);
	}
};

struct st_SECTOR_AROUND
{
	st_SECTOR_AROUND()
		: count(0)
	{

	}
	int count;
	st_Sector around[9];
};

#endif
