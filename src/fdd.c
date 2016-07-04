#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef DATA_FILE
#define DATA_FILE "data/budget.dat"
#endif

#ifndef CURRENCY
#define CURRENCY "EUR"
#endif

#define BREAKUP(n) (n) / 100, (n) % 100

static int verbose = 0;

static void usage(const char *argv) {
	printf("fdd -- Your personal fast food budget\n");
	printf("USAGE: %s [-i N] [-s N] -t\n", argv);
	printf("\tOn a new month:\n");
	printf("\t\t%s -i <money available per day (in cents)>\n", argv);
	printf("\tOtherwise:\n");
	printf("\t\t%s -s <spent amount (in cents)>\n", argv);
	printf("\t\t%s -t\tDisplays a summary\n", argv);
}

static int days_in_febuary(int year) {
	if(year % 400 == 0) return 29;
	if(year % 100 == 0) return 28;
	if(year % 4 == 0) return 29;
	return 28;
}

static struct tm *local_time(void) {
	time_t now = time(NULL);
	return localtime(&now);
}

static int days_this_month(void) {
	struct tm *stime;
	int ret;
	int daysinmonth[12] = { 
		31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 
	};

	stime = local_time();
	ret = daysinmonth[stime->tm_mon];

	if(stime->tm_mon == 1) {
		ret = days_in_febuary(stime->tm_year + 1900);
	}

	return ret;
}

static int write_db(int monthly_budget, int spent) {
	FILE *fp;

	if((fp = fopen(DATA_FILE, "w")) == NULL) {
		fprintf(stderr, "ERROR: Cannot open '%s'.\n", DATA_FILE);
		return EXIT_FAILURE;
	}

	fwrite(&monthly_budget, sizeof(int), 1, fp);
	fwrite(&spent, sizeof(int), 1, fp);
	fclose(fp);

	return EXIT_SUCCESS;
}

static int read_db(int *monthly_budget, int *spent) {
	FILE *fp;

	if((fp = fopen(DATA_FILE, "r")) == NULL) {
		fprintf(stderr, "ERROR: Cannot open '%s'.\n", DATA_FILE);
		return EXIT_FAILURE;
	}

	fread(monthly_budget, sizeof(int), 1, fp);
	fread(spent, sizeof(int), 1, fp);
	fclose(fp);

	return EXIT_SUCCESS;
}

static int init_month(const int daily_budget) {
	return write_db(daily_budget * days_this_month(), 0);
}

static int spend(const int amount) {
	int monthly_budget, spent;
	int ret;

	if((ret = read_db(&monthly_budget, &spent)) != EXIT_SUCCESS)
		return ret;

	spent += amount;
	
	if(verbose)
		printf("Spending of %d.%02d " CURRENCY " checked in.", BREAKUP(amount));
	else
		printf("OK;");


	if(spent > monthly_budget) {
		if(verbose) {
			printf(" You overspent your monthly budget!");
		} else {
			printf(" OVER;");
		}
	}
	printf("\n");

	return write_db(monthly_budget, spent);
}

static int print_budget(void) {
	struct tm *stime;
	int monthly_budget, daily_budget, spent, budget_today, remaining, next_day;
	int ret;

	if((ret = read_db(&monthly_budget, &spent)) != EXIT_SUCCESS)
		return ret;

	stime = local_time();
	daily_budget = monthly_budget / days_this_month();
	budget_today = monthly_budget * stime->tm_mday / days_this_month();
	remaining = budget_today - spent;

	if(remaining >= 0) {
		printf("%02d.%02d.%04d; %d;\n", 
			stime->tm_mday, stime->tm_mon + 1, stime->tm_year + 1900,
			remaining);
	} else if(spent < monthly_budget) {
		next_day = days_this_month() - (monthly_budget - spent) / daily_budget;
		if(spent % daily_budget == 0) next_day++;
		printf("%02d.%02d.%4d;\n", 
			next_day, stime->tm_mon + 1, stime->tm_year + 1900);
	} else {
		if(stime->tm_mon++ > 11) {
			stime->tm_mon = 0;
			stime->tm_year++;
		}
		printf("01.%02d.%4d;\n", 
				stime->tm_mon + 1, stime->tm_year + 1900);
	}

	return EXIT_SUCCESS;
}
static int print_budget_verbose(int summary) {
	struct tm *stime;
	int monthly_budget, daily_budget, spent, budget_today, remaining, next_day;
	int ret;

	if((ret = read_db(&monthly_budget, &spent)) != EXIT_SUCCESS)
		return ret;

	stime = local_time();
	daily_budget = monthly_budget / days_this_month();
	budget_today = monthly_budget * stime->tm_mday / days_this_month();
	remaining = budget_today - spent;

	if(summary) {
		printf("--------------------------------------\n");
		printf(" JUNK FOOD SUMMARY:\n");
		printf("--------------------------------------\n");
		printf(" Today is %02d.%02d.%4d.\n", 
				stime->tm_mday, stime->tm_mon + 1, stime->tm_year + 1900);
		printf(" Your monthly budget is: %d.%02d " CURRENCY "\n", 
				BREAKUP(monthly_budget));
		printf(" You spent:              %d.%02d " CURRENCY "\n", 
				BREAKUP(spent));
		printf(" Budget until today:     %d.%02d " CURRENCY "\n", 
				BREAKUP(budget_today));
		printf("--------------------------------------\n\n");
	}

	printf("You can spend ");
	if(remaining >= 0) {
		printf("%d.%02d " CURRENCY "\n\n", BREAKUP(remaining));
	} else if(spent < monthly_budget) {
		next_day = days_this_month() - (monthly_budget - spent) / daily_budget;
		if(spent % daily_budget == 0) next_day++;
		printf("again on %02d.%02d.%4d\n\n", 
				next_day, stime->tm_mon + 1, stime->tm_year + 1900);
	} else {
		printf("again next month.\n\n");
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
	int opt;
	int print_summary = 0;

	while((opt = getopt(argc, argv, "tvi:s:ht")) != -1) {
		switch(opt) {
			case 't': print_summary = 1; break;
			case 'v': verbose = 1; break;
			case 'i': init_month(atoi(optarg)); break;
			case 's': spend(atoi(optarg)); break;
			case 'h':
			default:
				usage(argv[0]);
				return EXIT_FAILURE;
				
		}
	}

	if(verbose)
		return print_budget_verbose(print_summary);
	else
		return print_budget();
}
