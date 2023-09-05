#pragma once

#include <HAPI/HAPI.h>
#include <HoudiniEngine/HoudiniApi.h>

#include <HitContext.h>
//#include <EditTool.h>
#include <Viewport.h>

namespace HoudiniEngine
{
    class HoudiniTerrain
    {
        public:
            
            HoudiniTerrain(const AZStd::vector<AZ::Vector3>& data);
            
            HoudiniTerrain(int rows, int cols, int cellSizeX, int cellSizeY)
                : m_rows(rows)
                , m_columns(cols)
                , m_cellSizeX(cellSizeX)
                , m_cellSizeY(cellSizeY)
            {

            }

            

            bool LoadData(const AZStd::vector<float>& data);
            double GetHeightAtXY(double x, double y);
            bool TransferToHeightmap();

            bool isValid(int x, int y)
            {
                bool valid = y >= 0 && y < m_data.size() && x >= 0 && x < m_data[y].size();
                return valid;
            }

            bool IsDataLoaded()
            {
                return m_dataLoaded;
            }

        private:
            int m_columns;
            int m_rows;

            double m_cellSizeX;
            double m_cellSizeY;            

            double m_maxElevation = 0;
            bool m_dataLoaded = false;

            AZStd::vector<AZStd::vector<double>> m_data;

    };
}
