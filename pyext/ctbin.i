/***************************************************************************
 *                      ctbin - CTA data binning tool                      *
 * ----------------------------------------------------------------------- *
 *  copyright (C) 2010 by Jurgen Knodlseder                                *
 * ----------------------------------------------------------------------- *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/**
 * @file ctbin.i
 * @brief CTA data binning tool SWIG definition
 * @author J. Knodlseder
 */
%{
/* Put headers and other declarations here that are needed for compilation */
#include "ctbin.hpp"
%}
%include gammalib.i


/***********************************************************************//**
 * @class ctbin
 *
 * @brief CTA data binning tool SWIG interface defintion.
 ***************************************************************************/
class ctbin : public GApplication  {
public:
    // Constructors and destructors
    ctbin(void) : GApplication(CTBIN_NAME, CTBIN_VERSION) { return; }

    // Methods
    int run(void);
};


/***********************************************************************//**
 * @brief CTA data binning tool SWIG extension
 ***************************************************************************/
%extend ctbin {
    ctbin copy() {
        return (*self);
    }
}