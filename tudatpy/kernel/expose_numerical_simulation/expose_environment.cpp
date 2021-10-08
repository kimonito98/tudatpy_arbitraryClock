/*    Copyright (c) 2010-2019, Delft University of Technology
 *    All rights reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#include "expose_environment.h"

#include "tudatpy/docstrings.h"

#include <tudat/astro/aerodynamics.h>
#include <tudat/astro/ephemerides.h>
#include <tudat/astro/gravitation.h>
#include "tudat/astro/ground_stations/groundStation.h"
#include "tudat/simulation/environment_setup/body.h"


#include <pybind11/chrono.h>
#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

//namespace py = pybind11;
//namespace tba = tudat::basic_astrodynamics;
//namespace tss = tudat::simulation_setup;
//namespace tp = tudat::propagators;
//namespace tinterp = tudat::interpolators;
//namespace te = tudat::ephemerides;
//namespace tni = tudat::numerical_integrators;
//namespace trf = tudat::reference_frames;
//namespace tmrf = tudat::root_finders;


namespace py = pybind11;

namespace tba = tudat::basic_astrodynamics;
namespace ta = tudat::aerodynamics;
namespace tr = tudat::reference_frames;
namespace te = tudat::ephemerides;
namespace tgs = tudat::ground_stations;
namespace tr = tudat::reference_frames;
namespace tg = tudat::gravitation;
namespace trf = tudat::reference_frames;
namespace tss = tudat::simulation_setup;



namespace tudat
{

namespace aerodynamics
{

double getTotalSurfaceArea( const std::shared_ptr< HypersonicLocalInclinationAnalysis > coefficientGenerator )
{
    double totalSurfaceArea = 0.0;
    for( int i = 0; i < coefficientGenerator->getNumberOfVehicleParts( ); i++ )
    {
        totalSurfaceArea += std::fabs( coefficientGenerator->getVehiclePart( i )->getTotalArea( ) );
    }
    return totalSurfaceArea;
}


//! Function that saves the vehicle mesh data used for a HypersonicLocalInclinationAnalysis to a file
std::pair< std::vector< Eigen::Vector3d >, std::vector< Eigen::Vector3d > > getVehicleMesh(
        const std::shared_ptr< HypersonicLocalInclinationAnalysis > localInclinationAnalysis )
{
    std::vector< boost::multi_array< Eigen::Vector3d, 2 > > meshPoints =
            localInclinationAnalysis->getMeshPoints( );
    std::vector< boost::multi_array< Eigen::Vector3d, 2 > > meshSurfaceNormals =
            localInclinationAnalysis->getPanelSurfaceNormals( );


    //    boost::array< int, 3 > independentVariables;
    //    independentVariables[ 0 ] = 0;
    //    independentVariables[ 1 ] = 6;
    //    independentVariables[ 2 ] = 0;

    //    std::vector< std::vector< std::vector< double > > > pressureCoefficients =
    //            localInclinationAnalysis->getPressureCoefficientList( independentVariables );

    int counter = 0;
    std::vector< Eigen::Vector3d > meshPointsList;
    std::vector< Eigen::Vector3d > surfaceNormalsList;
    //    std::map< int, Eigen::Vector1d > pressureCoefficientsList;

    for( unsigned int i = 0; i < meshPoints.size( ); i++ )
    {
        for( unsigned int j = 0; j < meshPoints.at( i ).shape( )[ 0 ] - 1; j++ )
        {
            for( unsigned int k = 0; k < meshPoints.at( i ).shape( )[ 1 ] - 1; k++ )
            {
                meshPointsList.push_back( meshPoints[ i ][ j ][ k ] );
                surfaceNormalsList.push_back( meshSurfaceNormals[ i ][ j ][ k ] );
                //                pressureCoefficientsList[ counter ] = ( Eigen::Vector1d( ) << pressureCoefficients[ i ][ j ][ k ] ).finished( );
                counter++;
            }
        }
    }

    return std::make_pair( meshPointsList, surfaceNormalsList );
}

}

}

namespace tudatpy {
namespace numerical_simulation {
namespace environment {

void expose_environment(py::module &m) {


    py::enum_<ta::AerodynamicCoefficientsIndependentVariables>(m, "AerodynamicCoefficientsIndependentVariables", "<no_doc>")
            .value("mach_number_dependent", ta::AerodynamicCoefficientsIndependentVariables::mach_number_dependent)
            .value("angle_of_attack_dependent", ta::AerodynamicCoefficientsIndependentVariables::angle_of_attack_dependent)
            .value("sideslip_angle_dependent", ta::AerodynamicCoefficientsIndependentVariables::angle_of_sideslip_dependent)
            .value("altitude_dependent", ta::AerodynamicCoefficientsIndependentVariables::altitude_dependent)
            .value("time_dependent", ta::AerodynamicCoefficientsIndependentVariables::time_dependent)
            .value("control_surface_deflection_dependent", ta::AerodynamicCoefficientsIndependentVariables::control_surface_deflection_dependent)
            .value("undefined_independent_variable", ta::AerodynamicCoefficientsIndependentVariables::undefined_independent_variable)
            .export_values();

    py::class_<ta::AerodynamicCoefficientInterface,
            std::shared_ptr<ta::AerodynamicCoefficientInterface>>(m, "AerodynamicCoefficientInterface" )
            .def_property_readonly("reference_area", &ta::AerodynamicCoefficientInterface::getReferenceArea )
            .def_property_readonly("current_force_coefficients", &ta::AerodynamicCoefficientInterface::getCurrentForceCoefficients )
            .def_property_readonly("current_moment_coefficients", &ta::AerodynamicCoefficientInterface::getCurrentMomentCoefficients )
            .def_property_readonly("current_coefficients", &ta::AerodynamicCoefficientInterface::getCurrentAerodynamicCoefficients )
            .def("update_coefficients", &ta::AerodynamicCoefficientInterface::updateCurrentCoefficients,
                 py::arg( "independent_variables" ),
                 py::arg( "time") );

    py::class_<ta::AerodynamicCoefficientGenerator<3, 6>,
            std::shared_ptr<ta::AerodynamicCoefficientGenerator<3, 6>>,
            ta::AerodynamicCoefficientInterface>(m, "AerodynamicCoefficientGenerator36", "<no_doc, only_dec>");

    m.def("get_default_local_inclination_mach_points", &ta::getDefaultHypersonicLocalInclinationMachPoints,
          py::arg( "mach_regime" ) = "Full" );

    m.def("get_default_local_inclination_angle_of_attack_points", &ta::getDefaultHypersonicLocalInclinationAngleOfAttackPoints );

    m.def("get_default_local_inclination_sideslip_angle_points", &ta::getDefaultHypersonicLocalInclinationAngleOfSideslipPoints );

    py::class_<ta::HypersonicLocalInclinationAnalysis,
            std::shared_ptr<ta::HypersonicLocalInclinationAnalysis>,
            ta::AerodynamicCoefficientGenerator<3, 6>>(m, "HypersonicLocalInclinationAnalysis" )
            .def(py::init<
                 const std::vector< std::vector< double > >&,
                 const std::shared_ptr< tudat::SurfaceGeometry >,
                 const std::vector< int >&,
                 const std::vector< int >&,
                 const std::vector< bool >&,
                 const std::vector< std::vector< int > >&,
                 const double,
                 const double,
                 const Eigen::Vector3d&,
                 const bool >(),
                 py::arg("independent_variable_points"),
                 py::arg("body_shape"),
                 py::arg("number_of_lines"),
                 py::arg("number_of_points"),
                 py::arg("invert_orders"),
                 py::arg("selected_methods"),
                 py::arg("reference_area"),
                 py::arg("reference_length"),
                 py::arg("moment_reference_point"),
                 py::arg("save_pressure_coefficients") = false );

    m.def("get_local_inclination_total_vehicle_area", &ta::getTotalSurfaceArea,
          py::arg( "local_inclination_analysis_object" ) );

    m.def("get_local_inclination_mesh", &ta::getVehicleMesh,
          py::arg( "local_inclination_analysis_object" ) );


    /*!
     **************   FLIGHT CONDITIONS AND ASSOCIATED FUNCTIONALITY  ******************
     */

    py::enum_<trf::AerodynamicsReferenceFrameAngles>(m, "AerodynamicsReferenceFrameAngles")
            .value("latitude_angle", trf::AerodynamicsReferenceFrameAngles::latitude_angle)
            .value("longitude_angle", trf::AerodynamicsReferenceFrameAngles::longitude_angle)
            .value("heading_angle", trf::AerodynamicsReferenceFrameAngles::heading_angle)
            .value("flight_path_angle", trf::AerodynamicsReferenceFrameAngles::flight_path_angle)
            .value("angle_of_attack", trf::AerodynamicsReferenceFrameAngles::angle_of_attack)
            .value("angle_of_sideslip", trf::AerodynamicsReferenceFrameAngles::angle_of_sideslip)
            .value("bank_angle", trf::AerodynamicsReferenceFrameAngles::bank_angle)
            .export_values();

    py::enum_<trf::AerodynamicsReferenceFrames>(m, "AerodynamicsReferenceFrames")
            .value("inertial_frame", trf::AerodynamicsReferenceFrames::inertial_frame)
            .value("corotating_frame", trf::AerodynamicsReferenceFrames::corotating_frame)
            .value("vertical_frame", trf::AerodynamicsReferenceFrames::vertical_frame)
            .value("trajectory_frame", trf::AerodynamicsReferenceFrames::trajectory_frame)
            .value("aerodynamic_frame", trf::AerodynamicsReferenceFrames::aerodynamic_frame)
            .value("body_frame", trf::AerodynamicsReferenceFrames::body_frame)
            .export_values();

    py::class_<trf::AerodynamicAngleCalculator,
            std::shared_ptr<trf::AerodynamicAngleCalculator>>(m, "AerodynamicAngleCalculator")
            .def("set_body_orientation_angle_functions",
                 py::overload_cast<
                 const std::function<double()>,
                 const std::function<double()>,
                 const std::function<double()>,
                 const std::function<void(const double)>>(
                     &trf::AerodynamicAngleCalculator::setOrientationAngleFunctions),
                 py::arg("angle_of_attack_function") = std::function<double()>(),       // <pybind11/functional.h>
                 py::arg("angle_of_sideslip_function") = std::function<double()>(),     // <pybind11/functional.h>
                 py::arg("bank_angle_function") = std::function<double()>(),            // <pybind11/functional.h>
                 py::arg("angle_update_function") = std::function<void(
                const double)>(),// <pybind11/functional.h>
                 "<no_doc>")
            .def("set_body_orientation_angles",
                 py::overload_cast<
                 const double,
                 const double,
                 const double>(&trf::AerodynamicAngleCalculator::setOrientationAngleFunctions),
                 py::arg("angle_of_attack") = TUDAT_NAN,
                 py::arg("angle_of_sideslip") = TUDAT_NAN,
                 py::arg("bank_angle") = TUDAT_NAN,
                 "<no_doc>")
            .def("get_rotation_matrix_between_frames",
                 &trf::AerodynamicAngleCalculator::getRotationMatrixBetweenFrames,
                 py::arg("original_frame"),
                 py::arg("target_frame"))
            .def("get_angle",
                 &trf::AerodynamicAngleCalculator::getAerodynamicAngle,
                 py::arg("angle_type"));


    py::class_<ta::FlightConditions,
            std::shared_ptr<ta::FlightConditions>>(m, "FlightConditions", get_docstring("FlightConditions").c_str())
//            .def(py::init<
//                 const std::shared_ptr<tudat::basic_astrodynamics::BodyShapeModel>,
//                 const std::shared_ptr<tudat::reference_frames::AerodynamicAngleCalculator>>(),
//                 py::arg("shape_model"),
//                 py::arg("aerodynamic_angle_calculator") = std::shared_ptr< tr::AerodynamicAngleCalculator>())
//            .def("update_conditions", &ta::FlightConditions::updateConditions, py::arg("current_time") )
            .def_property_readonly("aerodynamic_angle_calculator", &ta::FlightConditions::getAerodynamicAngleCalculator, get_docstring("FlightConditions.aerodynamic_angle_calculator").c_str())
            .def_property_readonly("longitude", &ta::FlightConditions::getCurrentLongitude, get_docstring("FlightConditions.longitude").c_str())
            .def_property_readonly("geodetic_latitude", &ta::FlightConditions::getCurrentGeodeticLatitude, get_docstring("FlightConditions.latitude").c_str())
            .def_property_readonly("time", &ta::FlightConditions::getCurrentTime, get_docstring("FlightConditions.time").c_str())
            .def_property_readonly("body_centered_body_fixed_state", &ta::FlightConditions::getCurrentBodyCenteredBodyFixedState, get_docstring("body_centered_body_fixed_state.time").c_str())
            .def_property_readonly("altitude", &ta::FlightConditions::getCurrentAltitude, get_docstring("altitude.time").c_str());

    py::class_<ta::AtmosphericFlightConditions,
            std::shared_ptr<ta::AtmosphericFlightConditions>,
            ta::FlightConditions>(m, "AtmosphericFlightConditions")
            .def_property_readonly("density", &ta::AtmosphericFlightConditions::getCurrentDensity, get_docstring("AtmosphericFlightConditions.density").c_str())
            .def_property_readonly("temperature", &ta::AtmosphericFlightConditions::getCurrentFreestreamTemperature, get_docstring("AtmosphericFlightConditions.temperature").c_str())
            .def_property_readonly("dynamic_pressure", &ta::AtmosphericFlightConditions::getCurrentDynamicPressure, get_docstring("AtmosphericFlightConditions.dynamic_pressure").c_str())
            .def_property_readonly("pressure", &ta::AtmosphericFlightConditions::getCurrentPressure, get_docstring("AtmosphericFlightConditions.pressure").c_str())
            .def_property_readonly("airspeed", &ta::AtmosphericFlightConditions::getCurrentAirspeed, get_docstring("AtmosphericFlightConditions.airspeed").c_str())
            .def_property_readonly("mach_number", &ta::AtmosphericFlightConditions::getCurrentMachNumber, get_docstring("AtmosphericFlightConditions.mach_number").c_str())
            .def_property_readonly("airspeed_velocity", &ta::AtmosphericFlightConditions::getCurrentAirspeedBasedVelocity, get_docstring("AtmosphericFlightConditions.airspeed_velocity").c_str())
            .def_property_readonly("speed_of_sound", &ta::AtmosphericFlightConditions::getCurrentSpeedOfSound, get_docstring("AtmosphericFlightConditions.speed_of_sound").c_str())
            .def_property_readonly("aero_coefficient_independent_variables",
                                   &ta::AtmosphericFlightConditions::getAerodynamicCoefficientIndependentVariables, get_docstring("AtmosphericFlightConditions.aero_coefficient_independent_variables").c_str())
            .def_property_readonly("control_surface_aero_coefficient_independent_variables",
                                   &ta::AtmosphericFlightConditions::getControlSurfaceAerodynamicCoefficientIndependentVariables, get_docstring("AtmosphericFlightConditions.control_surface_aero_coefficient_independent_variables").c_str())
            .def_property_readonly("aerodynamic_coefficient_interface", &ta::AtmosphericFlightConditions::getAerodynamicCoefficientInterface, get_docstring("AtmosphericFlightConditions.aerodynamic_coefficient_interface").c_str());



    /*!
     **************   EPHEMERIDES  ******************
     */

    py::class_<te::Ephemeris, std::shared_ptr<te::Ephemeris>>(m, "Ephemeris")
            .def("cartesian_state", &te::Ephemeris::getCartesianState, py::arg("seconds_since_epoch") = 0.0)
            .def("cartesian_position", &te::Ephemeris::getCartesianPosition, py::arg("seconds_since_epoch") = 0.0)
            .def("cartesian_velocity", &te::Ephemeris::getCartesianVelocity, py::arg("seconds_since_epoch") = 0.0);


    py::class_<te::ConstantEphemeris,
            std::shared_ptr<te::ConstantEphemeris>,
            te::Ephemeris>(
                m, "ConstantEphemeris")
            .def(py::init<
                 const std::function<Eigen::Vector6d()>,//<pybind11/functional.h>,<pybind11/eigen.h>
                 const std::string &,
                 const std::string &>(),
                 py::arg("constant_state_function"),
                 py::arg("reference_frame_origin") = "SSB",
                 py::arg("reference_frame_orientation") = "ECLIPJ2000")
            .def(py::init<
                 const Eigen::Vector6d,//<pybind11/eigen.h>
                 const std::string &,
                 const std::string &>(),
                 py::arg("constant_state"),
                 py::arg("reference_frame_origin") = "SSB",
                 py::arg("reference_frame_orientation") = "ECLIPJ2000")
            .def("update_constant_state", &te::ConstantEphemeris::updateConstantState,
                 py::arg("new_state"));

    py::class_<te::KeplerEphemeris,
            std::shared_ptr<te::KeplerEphemeris>,
            te::Ephemeris>(
                m, "KeplerEphemeris");

    py::class_<te::TabulatedCartesianEphemeris< double, double >,
            std::shared_ptr<te::TabulatedCartesianEphemeris< double, double > >,
            te::Ephemeris>(m, "TabulatedEphemeris")
            .def("reset_interpolator", &te::TabulatedCartesianEphemeris< double, double >::resetInterpolator,
                 py::arg("interpolator") );


    py::class_<te::Tle, std::shared_ptr<te::Tle>>(m, "Tle")
            .def(py::init<//ctor 1
                 const std::string &>(),
                 py::arg("lines"))
            .def(py::init<//ctor 2
                 const std::string &,
                 const std::string &>(),
                 py::arg("line_1"),
                 py::arg("line_2"))
            .def("get_epoch", &te::Tle::getEpoch)
            .def("get_b_star", &te::Tle::getBStar)
            .def("get_epoch", &te::Tle::getEpoch)
            .def("get_inclination", &te::Tle::getInclination)
            .def("get_right_ascension", &te::Tle::getRightAscension)
            .def("get_eccentricity", &te::Tle::getEccentricity)
            .def("get_arg_of_perigee", &te::Tle::getArgOfPerigee)
            .def("get_mean_anomaly", &te::Tle::getMeanAnomaly)
            .def("get_mean_motion", &te::Tle::getMeanMotion);

    py::class_<te::TleEphemeris,
            std::shared_ptr<te::TleEphemeris>,
            te::Ephemeris>(m, "TleEphemeris")
            .def(py::init<
                 const std::string &,
                 const std::string &,
                 const std::shared_ptr<te::Tle>,
                 const bool>(),
                 py::arg("frame_origin") = "Earth",
                 py::arg("frame_orientation") = "J2000",
                 py::arg("tle") = nullptr,
                 py::arg("use_sdp") = false);

    /*!
     **************   ROTATION MODELS  ******************
     */


    py::class_<te::RotationalEphemeris,
            std::shared_ptr<te::RotationalEphemeris>>
            RotationalEphemeris_(m, "RotationalEphemeris");


    m.def("transform_to_inertial_orientation",
          &te::transformStateToInertialOrientation<double, double>,
          py::arg("state_in_body_fixed_frame"),
          py::arg("current_time"),
          py::arg("rotational_ephemeris"));



    py::class_<te::LongitudeLibrationCalculator,
            std::shared_ptr<te::LongitudeLibrationCalculator>>(
                m, "LongitudeLibrationCalculator");

    py::class_<te::DirectLongitudeLibrationCalculator,
            std::shared_ptr<te::DirectLongitudeLibrationCalculator>,
            te::LongitudeLibrationCalculator>(
                m, "DirectLongitudeLibrationCalculator")
            .def(py::init< const double >(),
                 py::arg("scaled_libration_amplitude"));


    py::class_<te::SynchronousRotationalEphemeris,
            std::shared_ptr<te::SynchronousRotationalEphemeris>,
            te::RotationalEphemeris>(
                m, "SynchronousRotationalEphemeris")
            .def_property("libration_calculator",
                          &te::SynchronousRotationalEphemeris::getLongitudeLibrationCalculator,
                          &te::SynchronousRotationalEphemeris::setLibrationCalculation);

    /*!
     **************   GRAVITY FIELD  ******************
     */

    py::class_<tg::GravityFieldModel,
            std::shared_ptr<tg::GravityFieldModel>>(m, "GravityFieldModel")
            .def(py::init<
                 const double,
                 const std::function<void()>>(),
                 py::arg("gravitational_parameter"),
                 py::arg("update_inertia_tensor") = std::function<void()>()// <pybind11/functional.h>
            )
            .def("get_gravitational_parameter", &tg::GravityFieldModel::getGravitationalParameter)
            .def_property("gravitational_parameter", &tg::GravityFieldModel::getGravitationalParameter,
                          &tg::GravityFieldModel::resetGravitationalParameter);

    py::class_<tg::SphericalHarmonicsGravityField,
            std::shared_ptr<tg::SphericalHarmonicsGravityField >,
            tg::GravityFieldModel>(m, "SphericalHarmonicsGravityField")
            .def_property_readonly("reference_radius", &tg::SphericalHarmonicsGravityField::getReferenceRadius )
            .def_property_readonly("maximum_degree", &tg::SphericalHarmonicsGravityField::getDegreeOfExpansion )
            .def_property_readonly("maximum_order", &tg::SphericalHarmonicsGravityField::getOrderOfExpansion )
            .def_property("cosine_coefficients", &tg::SphericalHarmonicsGravityField::getCosineCoefficients,
                          &tg::SphericalHarmonicsGravityField::setCosineCoefficients)
            .def_property("sine_coefficients", &tg::SphericalHarmonicsGravityField::getSineCoefficients,
                          &tg::SphericalHarmonicsGravityField::setSineCoefficients);

    /*!
     **************   SHAPE MODELS  ******************
     */

    py::class_<tba::BodyShapeModel,
            std::shared_ptr<tba::BodyShapeModel>>(m, "ShapeModel")
            .def("get_average_radius", &tba::BodyShapeModel::getAverageRadius)
            .def_property_readonly("average_radius", &tba::BodyShapeModel::getAverageRadius);


    /*!
     **************   GROUND STATION FUNCTIONALITY  ******************
     */

    py::class_<tgs::GroundStation,
            std::shared_ptr<tgs::GroundStation>>(m, "GroundStation")
            .def_property_readonly("pointing_angles_calculator", &tgs::GroundStation::getPointingAnglesCalculator );

    py::class_<tgs::PointingAnglesCalculator,
            std::shared_ptr<tgs::PointingAnglesCalculator>>(m, "PointingAnglesCalculator")
            .def("calculate_elevation_angle", &tgs::PointingAnglesCalculator::calculateElevationAngle,
                 py::arg( "inertial_vector_to_target" ),
                 py::arg( "time" ) )
            .def("calculate_azimuth_angle", &tgs::PointingAnglesCalculator::calculateAzimuthAngle,
                 py::arg( "inertial_vector_to_target" ),
                 py::arg( "time" ) )
            .def("convert_inertial_vector_to_topocentric",
                 &tgs::PointingAnglesCalculator::convertVectorFromInertialToTopocentricFrame,
                 py::arg( "inertial_vector" ),
                 py::arg( "time" ) );


    /*!
     **************   BODY OBJECTS AND ASSOCIATED FUNCTIONALITY  ******************
     */

    py::class_<tss::Body, std::shared_ptr<tss::Body>>(m, "Body", get_docstring("Body").c_str())
            .def_property("ephemeris_frame_to_base_frame", &tss::Body::getEphemerisFrameToBaseFrame,
                          &tss::Body::setEphemerisFrameToBaseFrame)
            .def_property_readonly("state", &tss::Body::getState, get_docstring("Body.state").c_str())
            .def_property_readonly("position", &tss::Body::getPosition, get_docstring("Body.position").c_str())
            .def_property_readonly("velocity", &tss::Body::getVelocity, get_docstring("Body.velocity").c_str())
            .def_property_readonly("inertial_to_body_fixed_frame", &tss::Body::getCurrentRotationMatrixToLocalFrame, get_docstring("Body.inertial_to_body_fixed_frame").c_str())
            .def_property_readonly("body_fixed_to_inertial_frame", &tss::Body::getCurrentRotationMatrixToGlobalFrame, get_docstring("Body.body_fixed_to_inertial_frame").c_str())
            .def_property_readonly("inertial_to_body_fixed_frame_derivative", &tss::Body::getCurrentRotationMatrixDerivativeToLocalFrame, get_docstring("Body.inertial_to_body_fixed_frame_derivative").c_str())
            .def_property_readonly("body_fixed_to_inertial_frame_derivative", &tss::Body::getCurrentRotationMatrixDerivativeToGlobalFrame, get_docstring("Body.body_fixed_to_inertial_frame_derivative").c_str())
            .def_property_readonly("inertial_angular_velocity", &tss::Body::getCurrentAngularVelocityVectorInGlobalFrame, get_docstring("Body.inertial_angular_velocity").c_str())
            .def_property_readonly("body_fixed_angular_velocity", &tss::Body::getCurrentAngularVelocityVectorInLocalFrame, get_docstring("Body.body_fixed_angular_velocity").c_str())
            .def_property("mass", &tss::Body::getBodyMass, &tss::Body::setConstantBodyMass)
            .def_property("inertia_tensor", &tss::Body::getBodyInertiaTensor,
                          py::overload_cast<const Eigen::Matrix3d &>(
                              &tss::Body::setBodyInertiaTensor))
            .def_property_readonly("state_in_base_frame_from_ephemeris",
                 &tss::Body::getStateInBaseFrameFromEphemeris<double, double>)
            .def_property("ephemeris", &tss::Body::getEphemeris, &tss::Body::setEphemeris)
            .def_property("atmosphere_model", &tss::Body::getAtmosphereModel, &tss::Body::setAtmosphereModel)
            .def_property("shape_model", &tss::Body::getShapeModel, &tss::Body::setShapeModel)
            .def_property("gravity_field_model", &tss::Body::getGravityFieldModel, &tss::Body::setGravityFieldModel)
            .def_property("aerodynamic_coefficient_interface", &tss::Body::getAerodynamicCoefficientInterface,
                          &tss::Body::setAerodynamicCoefficientInterface)
            .def_property("flight_conditions", &tss::Body::getFlightConditions, &tss::Body::setFlightConditions)
            .def_property("rotation_model", &tss::Body::getRotationalEphemeris, &tss::Body::setRotationalEphemeris)
            .def_property_readonly("gravitational_parameter", &tss::Body::getGravitationalParameter);


    py::class_<tss::SystemOfBodies,
            std::shared_ptr<tss::SystemOfBodies> >(m, "SystemOfBodies", get_docstring("SystemOfBodies").c_str())
            .def("get", &tss::SystemOfBodies::getBody,
                 py::arg("body_name"),
                 get_docstring("SystemOfBodies.get").c_str())
            .def("create_empty_body", &tss::SystemOfBodies::createEmptyBody,
                 py::arg("body_name"),
                 py::arg("process_body") = 1,
                 get_docstring("SystemOfBodies.create_empty_body").c_str())
            .def("add_body", &tss::SystemOfBodies::addBody,
                 py::arg("body_to_add"),
                 py::arg("body_name"),
                 py::arg("process_body") = 1,
                 get_docstring("SystemOfBodies.add_body").c_str())
            .def("remove_body", &tss::SystemOfBodies::deleteBody,
                 py::arg("body_name"),
                 get_docstring("SystemOfBodies.remove_body").c_str());
//            .def_property_readonly("number_of_bodies", &tss::SystemOfBodies::getNumberOfBodies,
//                                   get_docstring("number_of_bodies").c_str() );

    /*!
     **************   SUPPORTING FUNCTIONS USED ENVIRONMENT MODELS  ******************
     */


}
}// namespace environment
}// namespace numerical_simulation
}// namespace tudatpy