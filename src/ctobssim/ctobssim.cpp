/***************************************************************************
 *                ctobssim - CTA observation simulator tool                *
 * ----------------------------------------------------------------------- *
 *  copyright (C) 2011 by Jurgen Knodlseder                                *
 * ----------------------------------------------------------------------- *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/**
 * @file ctobssim.cpp
 * @brief CTA observation simulator tool implementation
 * @author J. Knodlseder
 */

/* __ Includes ___________________________________________________________ */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "ctobssim.hpp"
#include "GTools.hpp"

/* __ Method name definitions ____________________________________________ */
#define G_SIMULATE_SOURCE       "ctobssim::simulate_source(GCTAObservation*,"\
                                                                " GPhotons&)"

/* __ Debug definitions __________________________________________________ */

/* __ Coding definitions _________________________________________________ */


/*==========================================================================
 =                                                                         =
 =                        Constructors/destructors                         =
 =                                                                         =
 ==========================================================================*/

/***********************************************************************//**
 * @brief Void constructor
 ***************************************************************************/
ctobssim::ctobssim(void) : GApplication(CTOBSSIM_NAME, CTOBSSIM_VERSION)
{
    // Initialise members
    init_members();

    // Write header into logger
    log_header();

    // Return
    return;
}


/***********************************************************************//**
 * @brief Observations constructor
 *
 * This constructor creates an instance of the class that is initialised from
 * an observation container.
 ***************************************************************************/
ctobssim::ctobssim(GObservations obs) : GApplication(CTOBSSIM_NAME, CTOBSSIM_VERSION)
{
    // Initialise members
    init_members();

    // Set observations
    m_obs = obs;

    // Write header into logger
    log_header();

    // Return
    return;
}


/***********************************************************************//**
 * @brief Command line constructor
 *
 * @param[in] argc Number of arguments in command line.
 * @param[in] argv Array of command line arguments.
 ***************************************************************************/
ctobssim::ctobssim(int argc, char *argv[]) : 
                    GApplication(CTOBSSIM_NAME, CTOBSSIM_VERSION, argc, argv)
{
    // Initialise members
    init_members();

    // Write header into logger
    log_header();

    // Return
    return;
}


/***********************************************************************//**
 * @brief Copy constructor
 *
 * @param[in] app Application.
 ***************************************************************************/
ctobssim::ctobssim(const ctobssim& app) : GApplication(app)
{
    // Initialise members
    init_members();

    // Copy members
    copy_members(app);

    // Return
    return;
}


/***********************************************************************//**
 * @brief Destructor
 ***************************************************************************/
ctobssim::~ctobssim(void)
{
    // Free members
    free_members();

    // Return
    return;
}


/*==========================================================================
 =                                                                         =
 =                               Operators                                 =
 =                                                                         =
 ==========================================================================*/

/***********************************************************************//**
 * @brief Assignment operator
 *
 * @param[in] app Application.
 ***************************************************************************/
ctobssim& ctobssim::operator= (const ctobssim& app)
{
    // Execute only if object is not identical
    if (this != &app) {

        // Copy base class members
        this->GApplication::operator=(app);

        // Free members
        free_members();

        // Initialise members
        init_members();

        // Copy members
        copy_members(app);

    } // endif: object was not identical

    // Return this object
    return *this;
}


/*==========================================================================
 =                                                                         =
 =                            Public methods                               =
 =                                                                         =
 ==========================================================================*/

/***********************************************************************//**
 * @brief Clear instance
 ***************************************************************************/
void ctobssim::clear(void)
{
    // Free members
    free_members();
    this->GApplication::free_members();

    // Initialise members
    this->GApplication::init_members();
    init_members();

    // Return
    return;
}


/***********************************************************************//**
 * @brief Execute application
 *
 * This method runs the simulation and saves the results.
 ***************************************************************************/
void ctobssim::execute(void)
{
    // Read ahead output filename so that it gets dumped correctly in the
    // parameters log
    m_outfile = par("outfile")->value();

    // Run the simulation
    run();

    // Save results
    save();

    // Return
    return;
}


/***********************************************************************//**
 * @brief Simulate event data
 *
 * This method runs the simulation. Results are not saved by this method.
 * Invoke "save" to save the results.
 ***************************************************************************/
void ctobssim::run(void)
{
    // Switch screen logging on in debug mode
    if (logDebug())
        log.cout(true);

    // Get parameters
    get_parameters();

    // Write input parameters into logger
    if (logTerse()) {
        log_parameters();
        log << std::endl;
    }

    // Write observation(s) into logger
    if (logTerse()) {
        log << std::endl;
        if (m_obs.size() > 1)
            log.header1("Observations");
        else
            log.header1("Observation");
        log << m_obs << std::endl;
    }

    // Write header
    if (logTerse()) {
        log << std::endl;
        if (m_obs.size() > 1)
            log.header1("Simulate observations");
        else
            log.header1("Simulate observation");
    }

    // Loop over all observation in the container
    for (int i = 0; i < m_obs.size(); ++i) {

        // Get CTA observation
        GCTAObservation* obs = dynamic_cast<GCTAObservation*>(m_obs(i));

        // Continue only if observation is a CTA observation
        if (obs != NULL) {

            // Write header for observation
            if (logTerse()) {
                if (obs->name().length() > 1)
                    log.header3("Observation "+obs->name());
                else
                    log.header3("Observation");
            }

            // Allocate CTA event list and attach it to the observation.
            // Note that the events() method removes any existing event
            // list before attaching a new event list.
            GCTAEventList* events = new GCTAEventList;
            obs->events(events);

            // Write header for photon simulation
            //if (logTerse())
            //    log.header3("Simulate photons");

            // Simulate photons
            GPhotons photons = simulate_photons(obs, m_obs.models());

            // Write header for sky event simulation
            //if (logTerse())
            //    log.header3("Simulate sky events");

            // Simulate source events
            simulate_source(obs, photons);

            // Write header for bacgkround event simulation
            //if (logTerse())
            //    log.header3("Simulate background events");

            // Simulate source events
            simulate_background(obs, m_obs.models());

        } // endif: CTA observation found

    } // endfor: looped over observations

    // Return
    return;
}


/***********************************************************************//**
 * @brief Save simulated observation
 *
 * This method saves the results.
 ***************************************************************************/
void ctobssim::save(void)
{
    // Write header
    if (logTerse()) {
        log << std::endl;
        if (m_obs.size() > 1)
            log.header1("Save observations");
        else
            log.header1("Save observation");
    }

    // Get output filename
    m_outfile = par("outfile")->value();

    // Loop over all observation in the container
    int file_num = 0;
    for (int i = 0; i < m_obs.size(); ++i) {

        // Get CTA observation
        GCTAObservation* obs = dynamic_cast<GCTAObservation*>(m_obs(i));

        // Save only if observation is a CTA observation
        if (obs != NULL) {

            // Set filename. If more than one file will be created an
            // index "_xxx" will be appended.
            std::string filename = m_outfile;
            if (file_num > 0)
                filename += "_"+str(file_num);

            // Save file
            obs->save(filename, clobber());

        } // endif: observation was a CTA observation

    } // endfor: looped over files

    // Return
    return;
}


/***********************************************************************//**
 * @brief Get application parameters
 *
 * Get all task parameters from parameter file or (if required) by querying
 * the user. Most parameters are only required if no observation exists so
 * far in the observation container. In this case, a single CTA observation
 * will be added to the container, using the definition provided in the
 * parameter file.
 ***************************************************************************/
void ctobssim::get_parameters(void)
{
    // If there are no observations in container then add a single CTA
    // observation using the parameters from the parameter file
    if (m_obs.size() == 0) {

        // Get CTA observation parameters
        m_infile = par("infile")->value();
        m_caldb  = par("caldb")->value();
        m_irf    = par("irf")->value();
        m_ra     = par("ra")->real();
        m_dec    = par("dec")->real();
        m_rad    = par("rad")->real();
        m_tmin   = par("tmin")->real();
        m_tmax   = par("tmax")->real();
        m_emin   = par("emin")->real();
        m_emax   = par("emax")->real();

        // Allocate CTA observation
        GCTAObservation obs;

        // Set pointing direction
        GCTAPointing pnt;
        GSkyDir      skydir;
        skydir.radec_deg(m_ra, m_dec);
        pnt.dir(skydir);

        // Set ROI
        GCTARoi     roi;
        GCTAInstDir instdir;
        instdir.radec_deg(m_ra, m_dec);
        roi.centre(instdir);
        roi.radius(m_rad);

        // Set GTI
        GGti  gti;
        GTime tstart;
        GTime tstop;
        tstart.met(m_tmin);
        tstop.met(m_tmax);
        gti.append(tstart, tstop);

        // Set energy boundaries
        GEbounds ebounds;
        GEnergy  emin;
        GEnergy  emax;
        emin.TeV(m_emin);
        emax.TeV(m_emax);
        ebounds.append(emin, emax);

        // Set CTA observation attributes
        obs.pointing(pnt);
        obs.response(m_irf, m_caldb);
        obs.roi(&roi);
        obs.gti(gti);
        obs.ebounds(ebounds);

        // Append CTA observation to container
        m_obs.append(obs);

        // Load models into container
        m_obs.models(m_infile);

    } // endif: there was no observation in the container

    // Get other parameters
    m_seed = par("seed")->integer();

    // Initialise random number generator
    m_ran.seed(m_seed);

    // Return
    return;
}


/***********************************************************************//**
 * @brief Simulate photons for a given CTA observation
 *
 * @param[in] obs Pointer on CTA observation.
 * @param[in] models Models.
 *
 * This method simulates photons for a given CTA observation. It will loop
 * over all energy boundaries, all Good Time Intervals and all sky models
 * to generate a list of photons.
 *
 * @todo Add margins to the data selections so that we generate also photons
 *       that fall slightly outside the ROI and the energy interval so that
 *       they can be scatter within the ROI and energy interval by the
 *       PSF and energy dispersion.
 ***************************************************************************/
GPhotons ctobssim::simulate_photons(const GCTAObservation* obs,
                                    const GModels&         models)
{
    // Initialise indentation for logging
    int indent = 0;

    // Extract MC parameters from observation
    const GCTARoi* roi = static_cast<const GCTARoi*>(obs->roi());
    GSkyDir        dir = roi->centre().skydir();
    double         rad = roi->radius();

    // Dump simulation cone information
    if (logNormal()) {
        log << parformat("Random number seed");
        log << str(m_seed) << std::endl;
        log << parformat("Simulation area");
        log << str(m_area) << " cm2" << std::endl;
        log << parformat("Simulation cone");
        log << "RA=" << dir.ra_deg() << " deg";
        log << ", Dec=" << dir.dec_deg() << "deg";
        log << ", r=" << rad << " deg" << std::endl;
    }

    // Allocate photons
    GPhotons photons;

    // Loop over all Good Time Intervals
    for (int it = 0; it <  obs->gti().size(); ++it) {

        // Extract time interval
        GTime tmin = obs->gti().tstart(it);
        GTime tmax = obs->gti().tstop(it);

        // Dump time interval
        if (logNormal()) {
            if (obs->gti().size() > 1) {
                indent++;
                log.indent(indent);
            }
            log << parformat("Time interval");
            log << tmin << " - " << tmax << std::endl;
        }

        // Loop over all energy boundaries
        for (int ie = 0; ie <  obs->ebounds().size(); ++ie) {

            // Extract energy boundaries
            GEnergy emin = obs->ebounds().emin(ie);
            GEnergy emax = obs->ebounds().emax(ie);

            // Dump energy range
            if (logNormal()) {
                if (obs->ebounds().size() > 1) {
                    indent++;
                    log.indent(indent);
                }
                log << parformat("Energy range");
                log << emin << " - " << emax << std::endl;
            }

            // Loop over all sky models
            for (int i = 0; i < models.size(); ++i) {

                // Get sky model (NULL if not a sky model)
                const GModelSky* model = dynamic_cast<const GModelSky*>(models(i));

                // If we have a sky model then simulate photons
                if (model != NULL) {

                    // Get photons
                    GPhotons p = model->mc(m_area, dir, rad, emin, emax,
                                           tmin, tmax, m_ran);

                    // Reserve new space for photons
                    photons.reserve(photons.size() + p.size());
        
                    // Add photons to list
                    for (int k = 0; k < p.size(); ++k) {
                        p[k].mcid(i);
                        photons.push_back(p[k]);
                    }

                    // Dump simulation result for model
                    if (logNormal()) {
                        log << parformat("MC source photons");
                        log << str(p.size());
                        if (model->name().length() > 0)
                            log << " [" << model->name() << "]";
                        log << std::endl;
                    }
                    
                } // endif: model was a sky model

            } // endfor: looped over models

        } // endfor: looped over all energy boundaries

    } // endfor: looped over all time intervals

    // Reset indentation
    log.indent(0);

    // Return photons
    return photons;
}


/***********************************************************************//**
 * @brief Simulate source events from photon list
 *
 * @param[in] obs Pointer on CTA observation.
 * @param[in] photons Photon list.
 *
 * @exception GCTAException::no_pointing
 *            No valid pointing found in CTA observation
 * @exception GCTAException::no_response
 *            No valid response found in CTA observation
 *
 * Simulate source events from a photon list for a given CTA observation.
 * The events are stored in as event list in the observation.
 *
 * This method does nothing if the observation pointer is NULL. It also
 * verifies if the observation has a valid pointing and response.
 ***************************************************************************/
void ctobssim::simulate_source(GCTAObservation* obs, const GPhotons& photons)
{
    // Continue only if observation pointer is valid
    if (obs != NULL) {

        // Get pointer on CTA pointing. As we have no time dependent
        // pointings implement we just determine the pointing from the
        // beginning of the observation. Throw an exception if the pointing
        // is not defined.
        GCTAPointing* pnt = obs->pointing(obs->tstart());
        if (pnt == NULL)
            throw GCTAException::no_pointing(G_SIMULATE_SOURCE);

        // Get pointer on CTA response. Throw an exception if the response
        // is not defined.
        GCTAResponse* rsp = obs->response();
        if (rsp == NULL)
            throw GCTAException::no_response(G_SIMULATE_SOURCE);

        // Make sure that the observation holds a CTA event list. If this
        // is not the case then allocate and attach a CTA event list now.
        if (dynamic_cast<const GCTAEventList*>(obs->events()) == NULL)
            obs->events(new GCTAEventList);
    
        // Get pointer on event list (circumvent const correctness)
        GCTAEventList* events = (GCTAEventList*)(obs->events());

        // Reserves space for source events in event list
        events->reserve(photons.size());

        // Simulate events from photons
        for (int i = 0; i < photons.size(); ++i) {
            GCTAEventAtom* event = rsp->mc(m_area, photons[i], *pnt, m_ran);
            if (event != NULL) {
                events->append(*event);
                delete event;
            }
        }

        // Dump simulation results
        if (logNormal()) {
            log << parformat("MC source events");
            log << str(events->size()) << std::endl;
        }

    } // endif: observation pointer was valid

    // Return
    return;
}


/***********************************************************************//**
 * @brief Simulate background events from model
 *
 * @param[in] obs Pointer on CTA observation.
 * @param[in] models Models.
 *
 * Simulate background events from models. The events are stored as event
 * list in the observation.
 *
 * This method does nothing if the observation pointer is NULL.
 ***************************************************************************/
void ctobssim::simulate_background(GCTAObservation* obs, const GModels& models)
{
    // Continue only if observation pointer is valid
    if (obs != NULL) {

        // Make sure that the observation holds a CTA event list. If this
        // is not the case then allocate and attach a CTA event list now.
        if (dynamic_cast<const GCTAEventList*>(obs->events()) == NULL)
            obs->events(new GCTAEventList);
    
        // Get pointer on event list (circumvent const correctness)
        GCTAEventList* events = (GCTAEventList*)(obs->events());

        // Loop over all models
        for (int i = 0; i < models.size(); ++i) {

            // Get model (NULL if not a radial acceptance model)
            const GCTAModelRadialAcceptance* model = 
                  dynamic_cast<const GCTAModelRadialAcceptance*>(models(i));

            // If we have a radial acceptance model then simulate events
            if (model != NULL) {

                // Get simulated event list
                GCTAEventList* list = model->mc(*obs, m_ran);

                // Reserves space for events
                events->reserve(list->size()+events->size());

                // Append events
                for (int k = 0; k < list->size(); k++)
                    events->append(*(list->pointer(k)));

                // Dump simulation results
                if (logNormal()) {
                    log << parformat("MC background events");
                    log << str(list->size()) << std::endl;
                }

                // Free event list
                delete list;

            } // endif: model was valid

        } // endfor: looped over all models

    } // endif: observation pointer was valid

    // Return
    return;
}


/***********************************************************************//**
 * @brief Save events
 *
 * @todo Set header keywords
 ***************************************************************************/
/*
void ctobssim::save_events(const GCTAEventList* events)
{
    // Allocate events file
    GFits file;

    // Write events
    events->write(&file);

    // Set keywords in EVENTS header
    set_events_keywords(&file);

    // Allocate Gti
    GGti gti;

    // Setup single GTI covering the selected time range
    GTime tstart;
    GTime tstop;
    tstart.met(m_tmin);
    tstop.met(m_tmax);
    gti.add(tstart, tstop);

    // Write GTI
    gti.write(&file);

    // Dump FITS file
    if (logExplicit()) {
        log << file << std::endl;
    }

    // Save events file. We need to use to saveto() method to make sure
    // that any existing file is indeed overwritten
    file.saveto(m_outfile, clobber());

    // Close FITS file
    file.close();

    // Return
    return;
}
*/

/***********************************************************************//**
 * @brief Set keywords of EVENTS header
 *
 * @param[in] file FITS file.
 ***************************************************************************/
/*
void ctobssim::set_events_keywords(GFits* file)
{
    // Get EVENTS HDU
    GFitsHDU* hdu = file->hdu("EVENTS");

    // Set observation time information
    hdu->card("TSTART",   m_tmin,        "[s] Mission time of start of observation");
    hdu->card("TSTOP",    m_tmax,        "[s] Mission time of end of observation");
    hdu->card("TELAPSE",  m_tmax-m_tmin, "[s] Mission elapsed time");
    hdu->card("ONTIME",   m_tmax-m_tmin, "[s] Total good time including deadtime");
    hdu->card("LIVETIME", m_tmax-m_tmin, "[s] Total livetime");

    // Set target information
    //hdu->card("RA_OBJ",  m_ra,  "[deg] Target Right Ascension");
    //hdu->card("DEC_OBJ", m_dec, "[deg] Target Declination");

    // Set pointing information
    hdu->card("RA_PNT",  m_ra,  "[deg] Pointing Right Ascension");
    hdu->card("DEC_PNT", m_dec, "[deg] Pointing Declination");

    // Return
    return;
}
*/

/*==========================================================================
 =                                                                         =
 =                             Private methods                             =
 =                                                                         =
 ==========================================================================*/

/***********************************************************************//**
 * @brief Initialise class members
 ***************************************************************************/
void ctobssim::init_members(void)
{
    // Initialise members
    m_infile.clear();
    m_outfile.clear();
    m_caldb.clear();
    m_irf.clear();
    m_obs.clear();
    m_seed =   1;
    m_ra   = 0.0;
    m_dec  = 0.0;
    m_rad  = 0.0;
    m_tmin = 0.0;
    m_tmax = 0.0;
    m_emin = 0.0;
    m_emax = 0.0;

    // Set fixed parameters
    m_area = 3200000.0 * 1.0e4; //!< 3200000.0 m^2

    // Set logger properties
    log.date(true);

    // Return
    return;
}


/***********************************************************************//**
 * @brief Copy class members
 *
 * @param[in] app Application.
 ***************************************************************************/
void ctobssim::copy_members(const ctobssim& app)
{
    // Copy attributes
    m_infile  = app.m_infile;
    m_outfile = app.m_outfile;
    m_caldb   = app.m_caldb;
    m_irf     = app.m_irf;
    m_obs     = app.m_obs;
    m_seed    = app.m_seed;
    m_ra      = app.m_ra;
    m_dec     = app.m_dec;
    m_rad     = app.m_rad;
    m_tmin    = app.m_tmin;
    m_tmax    = app.m_tmax;
    m_emin    = app.m_emin;
    m_emax    = app.m_emax;
    m_area    = app.m_area;

    // Return
    return;
}    


/***********************************************************************//**
 * @brief Delete class members
 ***************************************************************************/
void ctobssim::free_members(void)
{
    // Write separator into logger
    if (logTerse())
        log << std::endl;

    // Return
    return;
}
