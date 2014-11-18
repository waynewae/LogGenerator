#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include "libxl.h"

using namespace std;

int main(int argc, char* argv[])
//int main()
{
	char line[1000] = {0};
	string tmp;
	int test_pass = 1;
	string notifyTime;
	string lockinfo;
	string app_name;
	string lock_time;

	//string Date = "2014/11/17";
	//string SerailTest_xls = "20141117_serial_test_Report.xls";
	//string SerailTest_txt = "20141117_serial_test_Report.txt";
	//string BatteryStart = "20141117_battery_start.txt";
	//string BatteryEnd = "20141117_battery_end.txt";
	//string PMLog = "1415029862018_abnormalNotify@4316001.log";

	string SerailTest_xls = argv[1];
	string SerailTest_txt = argv[2];
	string BatteryStart = argv[3];
	string BatteryEnd = argv[4];
	string PMLog = argv[5];

	fstream WriteToHere;
	WriteToHere.open("DoU AutoTest report.html", ios::out | ios::trunc);
	if(!WriteToHere) cout << "Open DoU AutoTest report.html failed.\n";
	else cout << "Open DoU AutoTest report.html successful.\n\n";

	WriteToHere << "<!doctype html>\n"
				<< "<head>\n"
				<< "<meta http-equiv=Content-Type content=\"text/html; charset=utf8\">\n"
				<< "</head>\n"
				<< "<body>\n";

	// write Test information
	libxl::Book* book = xlCreateBook();
	if(book)
	{
		WriteToHere << "<h2>Information</h2>\n"
					<< "<pre>\n";

		if(book->load(SerailTest_xls.c_str()))
		{
			cout << "Open " << SerailTest_xls << " successful\n";
			libxl::Sheet* sheet = book->getSheet(0);
			if(sheet)
			{
				cout << "Load sheet successful\n";
				const char* imei = sheet->readStr(0, 0);
				WriteToHere << imei << ' ';
				const char* imei_val = sheet->readStr(0, 1);
				WriteToHere << imei_val << endl;
				const char* pro = sheet->readStr(1, 0);
				WriteToHere << pro << ' ';
				const char* pro_val = sheet->readStr(1, 1);
				WriteToHere << pro_val << endl;
				const char* android = sheet->readStr(2, 0);
				WriteToHere << android << ' ';
				const char* android_val = sheet->readStr(2, 1);
				WriteToHere << android_val << endl;
				const char* SW = sheet->readStr(3, 0);
				WriteToHere << SW << ' ';
				const char* SW_val = sheet->readStr(3, 1);
				WriteToHere << SW_val << endl;
				const char* start = sheet->readStr(4, 0);
				WriteToHere << start << ' ';
				const char* start_val = sheet->readStr(4, 1);
				WriteToHere << start_val << endl;
				const char* end = sheet->readStr(5, 0);
				WriteToHere << end << ' ';
				const char* end_val = sheet->readStr(5, 1);
				WriteToHere << end_val << endl;
			}
		}
		book->release();
	}
	WriteToHere << "</pre>\n"
				<< "<hr color=\"gray\">\n";


	// write serial test error
	fstream SerialTestError;
	SerialTestError.open(SerailTest_txt.c_str(), ios::in);
	if(!SerialTestError) cout << "Open " << SerailTest_txt << " failed.\n";
	else cout << "Open " << SerailTest_txt << " successful.\n";

	WriteToHere << "<h2>Error of AutoTest script</h2>\n"
				<< "<pre>\n";

	while(SerialTestError.getline(line, sizeof(line), '\n'))
	{
		tmp.assign(line, 0, 4);
		if (!tmp.compare("fail"))
		{
			WriteToHere << line << endl;
			test_pass = 0;
		}
	}
	if(test_pass == 1) WriteToHere << "N/A\n";
	WriteToHere << "</pre>\n"
				<< "<hr color=\"gray\">\n";
	SerialTestError.close();


	// write battery status
	fstream Start;
    Start.open(BatteryStart.c_str(), ios::in);
    if(!Start) cout << "Open " << BatteryStart << " failed.\n";
    else cout << "Open " << BatteryStart << " successful.\n";

	WriteToHere << "<h2>Battery Status</h2>\n"
				<< "<pre>"
				<< "<b>Start</b>\n";
	while(Start.getline(line, sizeof(line), '\n'))	WriteToHere << line << endl;
	Start.close();

	fstream End;
	End.open(BatteryEnd.c_str(), ios::in);
	if(!End) cout << "Open " << BatteryEnd << " failed.\n";
	else cout << "Open " << BatteryEnd << " successful.\n";

	WriteToHere << "<b>End</b>\n";
	while(End.getline(line, sizeof(line), '\n'))	WriteToHere << line << endl;
	End.close();

	WriteToHere << "</pre>\n"
				<< "<hr color=\"gray\">\n";

	// write abnormal power consumption
	WriteToHere << "<h3>Abnormal power consumption</h3>\n"
				<< "<pre>\n";
	fstream log;
	log.open(PMLog.c_str(), ios::in);
	if(!log)
	{
		cout << "Open " << PMLog << " failed.\n";
		WriteToHere << "N/A\n";
	}
	else
	{
		cout << "Open " << PMLog << " successful.\n";
		while(log.getline(line, sizeof(line), '\n'))
		{
			tmp.assign(line);

			// write timestamp
			if(!tmp.compare(0, 7, "warning"))
			{
				notifyTime = tmp.substr(8, 10);
				notifyTime.insert(2, "/");
				notifyTime.insert(5, " ");
				notifyTime.insert(8, ":");
				notifyTime.insert(11, ":");
				if(tmp.compare(19, 3, "END"))	WriteToHere << "<b>Start : ";
				else	WriteToHere << "<b>End : ";
				WriteToHere << notifyTime << "</b>" << endl;
			}

			// write wakelock information
			if(!tmp.compare(0, 4, "[uw]") || !tmp.compare(0, 4, "[kw]"))
			{
				if(!tmp.compare(0, 4, "[uw]"))
				{
					WriteToHere << "<b>User wakelocks</b>\n"
								<< "<table border=1>\n";
					tmp.erase(0, 4);
					while(tmp.length())
					{
						lockinfo.assign(tmp.substr(0, tmp.find(',')));
						tmp.erase(0, tmp.find(',') + 1);
						app_name.assign(lockinfo.substr(0, lockinfo.find_last_of('(')));
						app_name.erase(app_name.find('@'), app_name.length() - app_name.find('@'));
						lock_time.assign(lockinfo.substr(lockinfo.find_last_of('(') + 1, lockinfo.find_last_of(')') - lockinfo.find_last_of('(') - 1));
						WriteToHere << "<tr>\n"
									<< "<td>"<< app_name << "</td><td>" << lock_time << " min</td>\n"
									<< "</tr>\n";
					}
					WriteToHere << "</table>\n";
				}
				if(!tmp.compare(0, 4, "[kw]"))
				{
					WriteToHere << "<b>Kernel wakelocks</b>\n"
								<< "<table border=1>\n";
					tmp.erase(0, 4);
					while(tmp.length())
					{
						lockinfo.assign(tmp.substr(0, tmp.find(',')));
						tmp.erase(0, tmp.find(',') + 1);
						app_name.assign(lockinfo.substr(0, lockinfo.find_last_of('(')));
						lock_time.assign(lockinfo.substr(lockinfo.find_last_of('(') + 1, lockinfo.find_last_of(')') - lockinfo.find_last_of('(') - 1));
						WriteToHere << "<tr>\n"
									<< "<td>"<< app_name << "</td><td>" << lock_time << " min</td>\n"
									<< "</tr>\n";
					}
					WriteToHere << "</table>\n<br/>";
				}
			}
		}
	}
	log.close();
	WriteToHere << "</pre>\n";
	WriteToHere.close();
	return 0;
}
