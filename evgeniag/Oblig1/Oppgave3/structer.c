#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct datetime {
	unsigned int second, minute, hour;
  unsigned int date, month, year;
};

void init_datetime(struct datetime* dt);
void datetime_set_date(struct datetime* dt, unsigned int date, unsigned int month, unsigned int year);
void datetime_set_time(struct datetime* dt, unsigned int second, unsigned int minute, unsigned int hour);
void datetime_diff(struct datetime* dt_from,
                   struct datetime* dt_to,
                   struct datetime* dt_res);


int main(int argc, char const *argv[]) {
	struct datetime dt;
	init_datetime(&dt);
	datetime_set_date(&dt, 12, 9, 2016);
	datetime_set_time(&dt, 30, 27, 16);
	printf("%d.%d.%d  %d:%d:%d\n", dt.date, dt.month, dt.year, dt.hour, dt.minute, dt.second);

	struct datetime dt2;
	init_datetime(&dt2);
	datetime_set_date(&dt2, 12, 9, 2016);
	datetime_set_time(&dt2, 29, 27, 17);
	printf("%d.%d.%d  %d:%d:%d\n", dt2.date, dt2.month, dt2.year, dt2.hour, dt2.minute, dt2.second);

	struct datetime dt_res;
	datetime_diff(&dt, &dt2, &dt_res);
	printf("%d days %d months %d years  %d hours %d minutes %d seconds\n", dt_res.date, dt_res.month, dt_res.year, dt_res.hour, dt_res.minute, dt_res.second);

	return 0;
}

void init_datetime(struct datetime* dt){
	dt = malloc(sizeof(struct datetime));
}

void datetime_set_date(struct datetime* dt, unsigned int date, unsigned int month, unsigned int year){
	dt->date = date;
	dt->month = month;
	dt->year = year;
}

void datetime_set_time(struct datetime* dt, unsigned int second, unsigned int minute, unsigned int hour){
	dt->second = second;
	dt->minute = minute;
	dt->hour = hour;
}

void datetime_diff(struct datetime* dt_from,
										struct datetime* dt_to,
										struct datetime* dt_res){

		const unsigned int SEC_IN_MIN = 60;
		const unsigned int SEC_IN_HOUR = 3600;
		const unsigned int SEC_IN_DAY = SEC_IN_HOUR*24;
		const unsigned int SEC_IN_MONTH = SEC_IN_DAY*30;	//inaccurate with 31-moths
		const unsigned int SEC_IN_YEAR = SEC_IN_DAY*356; //inaccurate with leap-year

		//differense in seconds
		double dt_to_in_sec = (dt_to->second) + (dt_to->minute)*SEC_IN_MIN + (dt_to->hour)*SEC_IN_HOUR
								+ (dt_to->date)*SEC_IN_DAY + (dt_to->month)*SEC_IN_MONTH + (dt_to->year)*SEC_IN_YEAR;

		double dt_from_in_sec = (dt_from->second) + (dt_from->minute)*SEC_IN_MIN + (dt_from->hour)*SEC_IN_HOUR
								+ (dt_from->date)*SEC_IN_DAY + (dt_from->month)*SEC_IN_MONTH + (dt_from->year)*SEC_IN_YEAR;

		double diff_sec = dt_to_in_sec - dt_from_in_sec;



		int year_diff = (int)floor(diff_sec/(double)SEC_IN_YEAR);
		diff_sec = fmod(diff_sec,(double)SEC_IN_YEAR);

		int month_diff = (int)floor(diff_sec/(double)SEC_IN_MONTH);
		diff_sec = fmod(diff_sec,(double)SEC_IN_MONTH);

		int day_diff = (int)floor(diff_sec/(double)SEC_IN_DAY);
		diff_sec = fmod(diff_sec,(double)SEC_IN_DAY);

		int hour_diff = (int)floor(diff_sec/(double)SEC_IN_HOUR);
		diff_sec = fmod(diff_sec,(double)SEC_IN_HOUR);

		int minute_diff = (int)floor(diff_sec/(double)SEC_IN_MIN);
		diff_sec = fmod(diff_sec,(double)SEC_IN_MIN);

		int second_diff = diff_sec;

		datetime_set_date(dt_res, day_diff, month_diff, year_diff);
		datetime_set_time(dt_res, second_diff, minute_diff, hour_diff);

	}
