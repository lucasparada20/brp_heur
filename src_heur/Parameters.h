#ifndef _PARAMETERS_H
#define _PARAMETERS_H

#include <time.h>
#include <math.h>
#include <string>


class Parameters
{
	public:
		void Read(int arg, char ** argv);
		bool SetCityName(const char* filePath);

		static char* GetInstanceFileName(){return instance_file;}
		static char* GetInstanceType(){return instance_type;}
		static char* GetReFileName(){ return (char *)re_file_name.c_str(); }
		static char* GetOutputFileName(){ return (char *)output_file_name.c_str(); }
		static char* GetCityName() {return (char*)city_name.c_str();}		
		static int GetIterations(){ return Iterations; }
		static int GetNbStations(){ return nb_stations; }

	private:

		static char * instance_file;
		static char * instance_type;
		static std::string re_file_name;
		static std::string output_file_name;
		static std::string city_name;		
		
		static int Iterations;
		static int nb_stations;
};


#endif
