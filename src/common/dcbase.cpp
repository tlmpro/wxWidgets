/////////////////////////////////////////////////////////////////////////////
// Name:        common/dcbase.cpp
// Purpose:     generic methods of the wxDC Class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     05/25/99
// RCS-ID:      $Id$
// Copyright:   (c) wxWindows team
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

#ifdef __GNUG__
    #pragma implementation "dcbase.h"
#endif

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
   #include "wx/window.h"
#endif

#ifdef __WXMSW__
   #include "wx/msw/private.h"
#endif

#include "wx/dc.h"

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// special symbols
// ----------------------------------------------------------------------------

void wxDCBase::DoDrawCheckMark(wxCoord x1, wxCoord y1,
                               wxCoord width, wxCoord height)
{
    wxCHECK_RET( Ok(), wxT("invalid window dc") );

    wxCoord x2 = x1 + width,
            y2 = y1 + height;

    // this is to yield width of 3 for width == height == 10
    SetPen(wxPen(GetTextForeground(), (width + height + 1) / 7, wxSOLID));

    // we're drawing a scaled version of wx/generic/tick.xpm here
    wxCoord x3 = x1 + (4*width) / 10,   // x of the tick bottom
            y3 = y1 + height / 2;       // y of the left tick branch
    DoDrawLine(x1, y3, x3, y2);
    DoDrawLine(x3, y2, x2, y1);

    CalcBoundingBox(x1, y1);
    CalcBoundingBox(x2, y2);
}

// ----------------------------------------------------------------------------
// line/polygons
// ----------------------------------------------------------------------------

void wxDCBase::DrawLines(const wxList *list, wxCoord xoffset, wxCoord yoffset)
{
    int n = list->Number();
    wxPoint *points = new wxPoint[n];

    int i = 0;
    for ( wxNode *node = list->First(); node; node = node->Next(), i++ )
    {
        wxPoint *point = (wxPoint *)node->Data();
        points[i].x = point->x;
        points[i].y = point->y;
    }

    DoDrawLines(n, points, xoffset, yoffset);

    delete [] points;
}


void wxDCBase::DrawPolygon(const wxList *list,
                           wxCoord xoffset, wxCoord yoffset,
                           int fillStyle)
{
    int n = list->Number();
    wxPoint *points = new wxPoint[n];

    int i = 0;
    for ( wxNode *node = list->First(); node; node = node->Next(), i++ )
    {
        wxPoint *point = (wxPoint *)node->Data();
        points[i].x = point->x;
        points[i].y = point->y;
    }

    DoDrawPolygon(n, points, xoffset, yoffset, fillStyle);

    delete [] points;
}

// ----------------------------------------------------------------------------
// splines
// ----------------------------------------------------------------------------

#if wxUSE_SPLINES

// TODO: this API needs fixing (wxPointList, why (!const) "wxList *"?)
void wxDCBase::DrawSpline(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, wxCoord x3, wxCoord y3)
{
    wxList point_list;

    wxPoint *point1 = new wxPoint;
    point1->x = x1; point1->y = y1;
    point_list.Append((wxObject*)point1);

    wxPoint *point2 = new wxPoint;
    point2->x = x2; point2->y = y2;
    point_list.Append((wxObject*)point2);

    wxPoint *point3 = new wxPoint;
    point3->x = x3; point3->y = y3;
    point_list.Append((wxObject*)point3);

    DrawSpline(&point_list);

    for( wxNode *node = point_list.First(); node; node = node->Next() )
    {
        wxPoint *p = (wxPoint *)node->Data();
        delete p;
    }
}

void wxDCBase::DrawSpline(int n, wxPoint points[])
{
    wxList list;
    for (int i =0; i < n; i++)
    {
        list.Append((wxObject*)&points[i]);
    }

    DrawSpline(&list);
}

#endif // wxUSE_SPLINES

// ----------------------------------------------------------------------------
// enhanced text drawing
// ----------------------------------------------------------------------------

void wxDCBase::GetMultiLineTextExtent(const wxString& text,
                                      wxCoord *x,
                                      wxCoord *y,
                                      wxCoord *h,
                                      wxFont *font)
{
    int widthTextMax = 0, widthLine,
        heightTextTotal = 0, heightLineDefault = 0, heightLine = 0;

    wxString curLine;
    for ( const wxChar *pc = text; ; pc++ )
    {
        if ( *pc == _T('\n') || *pc == _T('\0') )
        {
            if ( curLine.empty() )
            {
                // we can't use GetTextExtent - it will return 0 for both width
                // and height and an empty line should count in height
                // calculation

                // assume that this line has the same height as the previous
                // one
                if ( !heightLineDefault )
                    heightLineDefault = heightLine;

                if ( !heightLineDefault )
                {
                    // but we don't know it yet - choose something reasonable
                    GetTextExtent(_T("W"), NULL, &heightLineDefault,
                                  NULL, NULL, font);
                }

                heightTextTotal += heightLineDefault;
            }
            else
            {
                GetTextExtent(curLine, &widthLine, &heightLine,
                              NULL, NULL, font);
                if ( widthLine > widthTextMax )
                    widthTextMax = widthLine;
                heightTextTotal += heightLine;
            }

            if ( *pc == _T('\n') )
            {
               curLine.clear();
            }
            else
            {
               // the end of string
               break;
            }
        }
        else
        {
            curLine += *pc;
        }
    }

    if ( x )
        *x = widthTextMax;
    if ( y )
        *y = heightTextTotal;
    if ( h )
        *h = heightLine;
}

void wxDCBase::DrawLabel(const wxString& text,
                         const wxBitmap& bitmap,
                         const wxRect& rect,
                         int alignment,
                         int indexAccel,
                         wxRect *rectBounding)
{
    // find the text position
    wxCoord width, heightText, heightLine;
    GetMultiLineTextExtent(text, &width, &heightText, &heightLine);

    wxCoord height;
    if ( bitmap.Ok() )
    {
        width += bitmap.GetWidth();
        height = bitmap.GetHeight();
    }
    else
    {
        height = heightText;
    }

    wxCoord x, y;
    if ( alignment & wxALIGN_RIGHT )
    {
        x = rect.GetRight() - width;
    }
    else if ( alignment & wxALIGN_CENTRE_HORIZONTAL )
    {
        x = (rect.GetLeft() + rect.GetRight() + 1 - width) / 2;
    }
    else // alignment & wxALIGN_LEFT
    {
        x = rect.GetLeft();
    }

    if ( alignment & wxALIGN_BOTTOM )
    {
        y = rect.GetBottom() - height;
    }
    else if ( alignment & wxALIGN_CENTRE_VERTICAL )
    {
        y = (rect.GetTop() + rect.GetBottom() + 1 - height) / 2;
    }
    else // alignment & wxALIGN_TOP
    {
        y = rect.GetTop();
    }

    // draw the bitmap first
    if ( bitmap.Ok() )
    {
        DrawBitmap(bitmap, x, y, TRUE /* use mask */);
        x += bitmap.GetWidth() + 4;
        y += (height - heightText) / 2;
    }

    // we will draw the underscore under the accel char later
    wxCoord startUnderscore = 0,
            endUnderscore = 0,
            yUnderscore = 0;

    // split the string into lines and draw each of them separately
    wxString curLine;
    for ( const wxChar *pc = text; ; pc++ )
    {
        if ( *pc == _T('\n') || *pc == _T('\0') )
        {
            if ( !curLine.empty() )
            {
                DrawText(curLine, x, y);
            }

            if ( *pc == _T('\0') )
                break;

            y += heightLine;

            curLine.clear();
        }
        else // not end of line
        {
            if ( pc - text == indexAccel )
            {
                // remeber to draw underscore here
                GetTextExtent(curLine, &startUnderscore, NULL);
                curLine += *pc;
                GetTextExtent(curLine, &endUnderscore, NULL);

                yUnderscore = y + heightLine;
            }
            else
            {
                curLine += *pc;
            }
        }
    }

    // draw the underscore if found
    if ( startUnderscore != endUnderscore )
    {
        // it should be of the same colour as text
        SetPen(wxPen(GetTextForeground(), 0, wxSOLID));

        yUnderscore--;

        DrawLine(x + startUnderscore, yUnderscore,
                 x + endUnderscore, yUnderscore);
    }

    // return bounding rect if requested
    if ( rectBounding )
    {
        *rectBounding = wxRect(x, y, width, height);
    }

    CalcBoundingBox(x, y);
    CalcBoundingBox(x + width, y + height);
}
