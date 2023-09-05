
#include "StdAfx.h"

#if defined(AZ_PLATFORM_WINDOWS)
/////////////////////////////////////////////////////////////////////////////
// Editor
/////////////////////////////////////////////////////////////////////////////
#include <EditorDefs.h>
//#include <EditTool.h> //O3DECONVERT
#include <Resource.h>
#endif

#include <HoudiniCommon.h>
#include "HoudiniPaintAttribTool.h"
#include "HoudiniTerrain.h"

#include <EditorCoreAPI.h>
//#include <Heightmap.h> //O3DECONVERT
//#include <TerrainManager.h> //O3DECONVERT

namespace HoudiniEngine
{
    HoudiniTerrain::HoudiniTerrain(const AZStd::vector<AZ::Vector3>& points)
    {
        AZ_PROFILE_FUNCTION(Editor);
        double width = 0;
        double height = 0;
        
        //Find the dimensions
        float xLast = points[0].GetX();
        float yLast = points[0].GetY();
        
        for (auto& point : points)
        {
            float curX = point.GetX();
            float curY = point.GetY();

            float diffX = curX - xLast;
            float diffY = curY - yLast;

            if (diffX > 0.1f)
            {
                m_cellSizeX = diffX;
            }

            if (diffY > 0.1f)
            {
                m_cellSizeY = diffY;
            }

            if (curX > width)
                width = curX;

            if (curY > height)
                height = curY;

            xLast = curX;
            yLast = curY;
        }

        if (width > 0 && height > 0)
        {
            m_rows = width / m_cellSizeX;
            m_columns = height / m_cellSizeY;

            //Account for the 0th row and col;
            m_rows++;
            m_columns++;

            if (m_rows * m_columns == points.size())
            {
                AZStd::vector<float> heightData;
                heightData.reserve(points.size());

                //convert to height field
                for (auto& point : points)
                {
                    heightData.push_back(point.GetZ());
                }

                LoadData(heightData);
            }
        }
    }


    bool HoudiniTerrain::LoadData(const AZStd::vector<float>& data)
    {
        AZ_PROFILE_FUNCTION(Editor);

        m_data = AZStd::vector<AZStd::vector<double>>(m_rows);   // reset data & allocate necessary size
        for (int i = 0; i < m_rows; i++)
        {
            m_data[i].resize(m_columns);
        }

        m_maxElevation = 0;
        
        if (data.size() != m_rows * m_columns)
        {
            return false;
        }

        for (int r = 0; r < m_rows; r++)
        {
            for (int c = 0; c < m_columns; c++)
            {
                int index = (r * m_columns + c);
                double value = (double)data[index];
                m_data[r][c] = value;
                
                if (value > m_maxElevation)
                {
                    m_maxElevation = value;
                }
            }
        }

        return true;
    }

    double HoudiniTerrain::GetHeightAtXY(double xRel, double yRel)
    {
        AZ_PROFILE_FUNCTION(Editor);

        double height = 0;

        if (!m_data.empty())
        {
            double dX = xRel / m_cellSizeX;     // x position within the grid, based on x size of grid cells
            double dY = yRel / m_cellSizeY;     // y position within the grid, based on y size of grid cells

            int x = (int)floor(dX);     // floored x position in the grid
            int y = (int)floor(dY);     // floored y position in the grid

            double alphaX = dX - x;     // position of dX between x and x + 1
            double alphaY = dY - y;     // position of dY between y and y + 1

            Limit(y, 0, (int)m_data.size() - 1);        // prevent y from going out of bounds (near miss issue)
            Limit(x, 0, (int)m_data[y].size() - 1);     // prevent x from going out of bounds (near miss issue)

            int x0 = x + 1;
            int y0 = y + 1;

            double h0, h1, hX, hY = 0;

            if (isValid(x, y))
            {
                h0 = m_data[y][x];
            }
            else
            {
                return height;
            }

            // Get most accurate height data for OOB positions (prevents jagged map boarders)
            if (isValid(x0, y0))
            {
                hY = m_data[y0][x];
                hX = m_data[y][x0];
                h1 = m_data[y0][x0];
            }
            else if (isValid(x0, y))
            {
                hY = h0;
                hX = h1 = m_data[y][x0];
            }
            else if (isValid(x, y0))
            {
                hY = h1 = m_data[y0][x];
                hX = h0;
            }
            else
            {
                hY = hX = h1 = h0;
            }

            double a00 = h0;
            double a10 = hX - h0;
            double a01 = hY - h0;
            double a11 = h1 + h0 - (hX + hY);

            height = a00 + (a10 * alphaX) + (a01 * alphaY) + (a11 * alphaX * alphaY);
        }

        return height;
    }

    bool HoudiniTerrain::TransferToHeightmap()
    {
        AZ_PROFILE_FUNCTION(Editor);

        //TODO
        // O3DECONVERT
        /*CHeightmap* heightmap = GetIEditor()->GetHeightmap();
        if (!heightmap->GetUseTerrain())
        {
            return false;
        }

        if (!GetIEditor()->IsUndoRecording())
        {
            GetIEditor()->BeginUndo();
        }

        heightmap->RecordAzUndoBatchTerrainModify(0, 0, heightmap->GetWidth(), heightmap->GetHeight());

        //heightmap->SetMaxHeight(m_maxElevation);
        int unitSize = heightmap->GetUnitSize();

        for (int x = 0; x < heightmap->GetWidth(); x++)
        {
            int xUnitSize = x * unitSize;
            for (int y = 0; y < heightmap->GetHeight(); y++)
            {                
                int yUnitSize = y * unitSize;

                float elevation = GetHeightAtXY(xUnitSize, yUnitSize);
                heightmap->SetXY(y, x, elevation);
            }
        }

        if (GetIEditor()->IsUndoRecording())
        {
            GetIEditor()->AcceptUndo("Terrain Modify");
        }

        heightmap->UpdateEngineTerrain(0, 0, heightmap->GetWidth(), heightmap->GetHeight(), true, false);*/
        return true;
    }


}
