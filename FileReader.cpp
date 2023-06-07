#include "FileReader.h"
#include <iostream>
#include <iterator>

const size_t FileReader::NUM_RECS_READ=7;

FileReader::FileReader(const std::string &fileName)
{
    dataFile_.open(fileName, NcFile::read);

    latVar_ = dataFile_.getVar("lat");
    if (latVar_.isNull())
        cout << "Error: Unable to read latitude data" << endl;

    longVar_ = dataFile_.getVar("lon");
    if (latVar_.isNull())
        cout << "Error: Unable to read longitude data" << endl;

    timeVar_ = dataFile_.getVar("time");
    if (timeVar_.isNull())
        cout << "Error: Unable to read time data" << endl;

    precipVar_ = dataFile_.getVar("precip");
    if (precipVar_.isNull())
        cout << "Error: Unable to read precipitation data" << endl;

    interpErrorVar_ = dataFile_.getVar("interpolation_error");
    if (interpErrorVar_.isNull())
        cout << "Error: Unable to read interpolation error data" << endl;

    numGaugeVar_ = dataFile_.getVar("numgauge");
    if (numGaugeVar_.isNull())
        cout << "Error: Unable to read numguage data" << endl;
}

void FileReader::getLatitude(double *dataValues)
{
    latVar_.getVar(dataValues);
}

void FileReader::getLongitude(double *dataValues)
{
    longVar_.getVar(dataValues);
}

void FileReader::getTime(double *dataValues)
{
    timeVar_.getVar(dataValues);
}

void FileReader::getPrecipitation(float *dataValues, size_t numLat, size_t numLon, size_t day)
{
    size_t num_recs_read = (day + NUM_RECS_READ < 365)? NUM_RECS_READ : 365-day;
    vector<size_t> start{day, 0, 0};
    vector<size_t> count{num_recs_read, numLat, numLon};


    try {
        precipVar_.getVar(start, count, dataValues);
    }
    catch(netCDF::exceptions::NcEdge)
    {
        std::cerr << "Trying to access a day in the dataset that does not exist.\nIgnoring and continuing..." << std::endl;
    }

}

Array1DPtr FileReader::filterData(float *array, int numLat, int numLon, bool filterZeros)
{
    auto tempArray = Array1DPtr{new Array1D{numLat * numLon * (int) NUM_RECS_READ}};

    float *end = array + numLat * numLon;

    for (float *p = array; p != end; ++p)
    {
        if (filterZeros)
        {
            if (*p != -9999.f && !(*p - 0.f < 0.0000001f))
            {
                tempArray->setNext(*p);
                // cout << *p << endl;
            }
        }
        else
        {
            if (*p != -9999.f)
            {
                tempArray->setNext(*p);
                // cout << *p << endl;
            }
        }
    }

    int idx = tempArray->sizeNoFiller();
    return Array1DPtr{new Array1D{idx - 1, *tempArray}};
}