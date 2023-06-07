#ifndef FILE_READER_H
#define FILE_READER_H

#include "Array1D.h"
#include <netcdf>
#include <string>
#include <memory>

using Array1DPtr = std::shared_ptr<Array1D>;

using namespace std;
using namespace netCDF;

class FileReader
{
public:
    FileReader(const string &fileName);

    void getLatitude(double *dataValues);
    void getLongitude(double *dataValues);
    void getTime(double *dataValues);
    void getPrecipitation(float *dataValues, size_t numLat, size_t numLon, size_t day);

    Array1DPtr filterData(float *array, int numLat, int numLon, bool filterZeros = false);

    static const size_t NUM_RECS_READ;

private:
    NcFile dataFile_;

    NcVar latVar_;
    NcVar longVar_;
    NcVar timeVar_;
    NcVar precipVar_;
    NcVar interpErrorVar_;
    NcVar numGaugeVar_;
};

#endif // FILE_READER_H