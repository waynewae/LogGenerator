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

//	string Date = "2014/11/17";
//	string SerailTest_xls = "20141117_serial_test_Report.xls";
//	string SerailTest_txt = "20141117_serial_test_Report.txt";
//	string BatteryStart = "20141117_battery_start.txt";
//	string BatteryEnd = "20141117_battery_end.txt";
//	string PMLog = "1415029862018_abnormalNotify@4316001.log";

	string device = argv[1];
	string SerailTest_xls = argv[2];
	string SerailTest_txt = argv[3];
	string BatteryStart = argv[4];
	string BatteryEnd = argv[5];

	fstream WriteToHere;
	WriteToHere.open("DoU_AutoTest_report.txt", ios::out | ios::trunc);
	if(!WriteToHere) cout << "Open DoU_AutoTest_report.html failed.\n";
	else cout << "Open DoU_AutoTest_report.html successful.\n\n";

//	WriteToHere << "<!doctype html>\n"
//				<< "<head>\n"
//				<< "<meta http-equiv=Content-Type content=\"text/html; charset=utf8\">\n"
//				<< "</head>\n"
//				<< "<body>\n";

	// write Test information
	libxl::Book* book = xlCreateBook();
	if(book)
	{
		WriteToHere << "Information\n"
					<< "Device: " << device << endl;

		if(book->load(SerailTest_xls.c_str()))
		{
			cout << "Open " << SerailTest_xls << " successful\n";
			libxl::Sheet* sheet = book->getSheet(0);
			if(sheet)
			{
				cout << "Load sheet successful\n";
				const char* imei = sheet->readStr(0, 0);
				WriteToHere << imei;
				const char* imei_val = sheet->readStr(0, 1);
				WriteToHere << ' ' << imei_val << endl;
				const char* pro = sheet->readStr(1, 0);
				WriteToHere << pro;
				const char* pro_val = sheet->readStr(1, 1);
				WriteToHere << ' ' << pro_val << endl;
				const char* android = sheet->readStr(2, 0);
				WriteToHere << android;
				const char* android_val = sheet->readStr(2, 1);
				WriteToHere << ' ' << android_val << endl;
				const char* SW = sheet->readStr(3, 0);
				WriteToHere << SW;
				const char* SW_val = sheet->readStr(3, 1);
				WriteToHere << ' ' << SW_val << endl;
				const char* start = sheet->readStr(4, 0);
			}
		}
		book->release();
	}


	// write battery status
	fstream Start;
    Start.open(BatteryStart.c_str(), ios::in);
    if(!Start) cout << "Open " << BatteryStart << " failed.\n";
    else cout << "Open " << BatteryStart << " successful.\n";

	WriteToHere << "Battery Status\n"
				<< "Start,";
	while(Start.getline(line, sizeof(line), '\n'))	WriteToHere << line << endl;
	Start.close();

	fstream End;
	End.open(BatteryEnd.c_str(), ios::in);
	if(!End) cout << "Open " << BatteryEnd << " failed.\n";
	else cout << "Open " << BatteryEnd << " successful.\n";

	WriteToHere << "End,";
	while(End.getline(line, sizeof(line), '\n'))	WriteToHere << line << endl;
	End.close();

	// write serial test error
	fstream SerialTestError;
	SerialTestError.open(SerailTest_txt.c_str(), ios::in);
	if(!SerialTestError) cout << "Open " << SerailTest_txt << " failed.\n";
	else cout << "Open " << SerailTest_txt << " successful.\n";

	WriteToHere << "Error of AutoTest script\n";

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
	SerialTestError.close();


	// write abnormal power consumption
	WriteToHere << "Abnormal power consumption\n";
	if(argv[6])
//	if(!PMLog.empty())
	{
		string PMLog = argv[6];
		fstream log;
		log.open(PMLog.c_str(), ios::in);
		if(!log)	cout << "Open " << PMLog << " failed.\n";
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
					if(tmp.compare(19, 3, "END"))	WriteToHere << "Start,";
					else	WriteToHere << "End,";
					WriteToHere << notifyTime << endl;
				}

				// write wakelock information
				if(!tmp.compare(0, 4, "[uw]") || !tmp.compare(0, 4, "[kw]"))
				{
					if(!tmp.compare(0, 4, "[uw]"))
					{
						WriteToHere << "User wakelocks\n";
						tmp.erase(0, 4);
						while(tmp.length())
						{
							lockinfo.assign(tmp.substr(0, tmp.find(',')));
							tmp.erase(0, tmp.find(',') + 1);
							app_name.assign(lockinfo.substr(0, lockinfo.find_last_of('(')));
							app_name.erase(app_name.find('@'), app_name.length() - app_name.find('@'));
							lock_time.assign(lockinfo.substr(lockinfo.find_last_of('(') + 1, lockinfo.find_last_of(')') - lockinfo.find_last_of('(') - 1));
							WriteToHere << app_name << "," << lock_time << " min\n";
						}
					}
					if(!tmp.compare(0, 4, "[kw]"))
					{
						WriteToHere << "Kernel wakelocks\n";
						tmp.erase(0, 4);
						while(tmp.length())
						{
							lockinfo.assign(tmp.substr(0, tmp.find(',')));
							tmp.erase(0, tmp.find(',') + 1);
							app_name.assign(lockinfo.substr(0, lockinfo.find_last_of('(')));
							lock_time.assign(lockinfo.substr(lockinfo.find_last_of('(') + 1, lockinfo.find_last_of(')') - lockinfo.find_last_of('(') - 1));
							WriteToHere << app_name << "," << lock_time << " min\n";
						}
					}
				}
			}
		}
		log.close();
	}
	else
	{
		WriteToHere << "N/A\n";
	}
		WriteToHere.close();
	return 0;
}
