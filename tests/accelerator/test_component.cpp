#include <gtest/gtest.h>
#include <cmath>

#include "accelerator/Component.hpp"
#include "physics/Constants.hpp"

namespace pas::accelerator::tests {

using namespace physics::constants;

class ComponentTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-10;
};

// Aperture tests

TEST_F(ComponentTest, CircularApertureContainsCenter) {
    Aperture aperture;
    aperture.shape = ApertureShape::Circular;
    aperture.radiusX = 0.05;

    EXPECT_TRUE(aperture.isInside(0.0, 0.0));
    EXPECT_TRUE(aperture.isInside(0.03, 0.03));
    EXPECT_FALSE(aperture.isInside(0.05, 0.05));
}

TEST_F(ComponentTest, RectangularApertureContainsCorners) {
    Aperture aperture;
    aperture.shape = ApertureShape::Rectangular;
    aperture.radiusX = 0.1;
    aperture.radiusY = 0.05;

    EXPECT_TRUE(aperture.isInside(0.0, 0.0));
    EXPECT_TRUE(aperture.isInside(0.1, 0.05));
    EXPECT_FALSE(aperture.isInside(0.11, 0.0));
}

TEST_F(ComponentTest, EllipticalApertureShape) {
    Aperture aperture;
    aperture.shape = ApertureShape::Elliptical;
    aperture.radiusX = 0.1;
    aperture.radiusY = 0.05;

    EXPECT_TRUE(aperture.isInside(0.0, 0.0));
    EXPECT_TRUE(aperture.isInside(0.1, 0.0));
    EXPECT_TRUE(aperture.isInside(0.0, 0.05));
    EXPECT_FALSE(aperture.isInside(0.1, 0.05));  // Corner of ellipse is outside
}

// BeamPipe tests

TEST_F(ComponentTest, BeamPipeHasNoField) {
    BeamPipe pipe("TestPipe", 1.0);

    EXPECT_EQ(pipe.getType(), ComponentType::BeamPipe);
    EXPECT_EQ(pipe.getFieldSource(), nullptr);
    EXPECT_EQ(pipe.getLength(), 1.0);
}

TEST_F(ComponentTest, BeamPipeTypeName) {
    BeamPipe pipe("TestPipe", 1.0);
    EXPECT_EQ(pipe.getTypeName(), "BeamPipe");
}

// Dipole tests

TEST_F(ComponentTest, DipoleHasCorrectField) {
    Dipole dipole("TestDipole", 2.0, 1.5);

    EXPECT_EQ(dipole.getType(), ComponentType::Dipole);
    EXPECT_EQ(dipole.getLength(), 2.0);
    EXPECT_DOUBLE_EQ(dipole.getField(), 1.5);
    EXPECT_NE(dipole.getFieldSource(), nullptr);
}

TEST_F(ComponentTest, DipoleBendingAngle) {
    Dipole dipole("TestDipole", 1.0, 1.0);

    // For a 1 GeV proton
    double kineticEnergy = 1.0 * energy::GeV;
    double gamma = relativistic::gammaFromKineticEnergy(kineticEnergy, m_p);
    double beta = relativistic::betaFromGamma(gamma);
    double momentum = gamma * beta * m_p * c;

    double angle = dipole.getBendingAngle(momentum);

    // theta = q * B * L / p
    double expected = e * 1.0 * 1.0 / momentum;
    EXPECT_NEAR(angle, expected, EPSILON);
}

TEST_F(ComponentTest, DipoleBendingRadius) {
    Dipole dipole("TestDipole", 1.0, 1.0);

    double momentum = 1e-18;  // Arbitrary momentum
    double radius = dipole.getBendingRadius(momentum);

    // rho = p / (q * B)
    double expected = momentum / (e * 1.0);
    EXPECT_NEAR(radius, expected, EPSILON);
}

TEST_F(ComponentTest, DipoleFieldCanBeChanged) {
    Dipole dipole("TestDipole", 1.0, 1.0);

    dipole.setField(2.5);
    EXPECT_DOUBLE_EQ(dipole.getField(), 2.5);
}

// Quadrupole tests

TEST_F(ComponentTest, QuadrupoleHasCorrectGradient) {
    Quadrupole quad("TestQuad", 0.5, 50.0);

    EXPECT_EQ(quad.getType(), ComponentType::Quadrupole);
    EXPECT_EQ(quad.getLength(), 0.5);
    EXPECT_DOUBLE_EQ(quad.getGradient(), 50.0);
}

TEST_F(ComponentTest, QuadrupoleFocusing) {
    Quadrupole qf("QF", 0.5, 50.0);
    Quadrupole qd("QD", 0.5, -50.0);

    EXPECT_TRUE(qf.isFocusing());
    EXPECT_FALSE(qd.isFocusing());
}

TEST_F(ComponentTest, QuadrupoleK1Calculation) {
    Quadrupole quad("TestQuad", 0.5, 100.0);

    double momentum = 1e-18;
    double k1 = quad.getK1(momentum);

    // K1 = q * G / p
    double expected = e * 100.0 / momentum;
    EXPECT_NEAR(k1, expected, EPSILON);
}

TEST_F(ComponentTest, QuadrupoleFieldSource) {
    Quadrupole quad("TestQuad", 0.5, 50.0);

    auto field = quad.getFieldSource();
    EXPECT_NE(field, nullptr);
}

// RFCavity tests

TEST_F(ComponentTest, RFCavityHasCorrectParameters) {
    RFCavity cavity("TestCavity", 0.5, 1e6, 400e6, 0.0);

    EXPECT_EQ(cavity.getType(), ComponentType::RFCavity);
    EXPECT_DOUBLE_EQ(cavity.getVoltage(), 1e6);
    EXPECT_DOUBLE_EQ(cavity.getFrequency(), 400e6);
    EXPECT_DOUBLE_EQ(cavity.getPhase(), 0.0);
}

TEST_F(ComponentTest, RFCavityEnergyGain) {
    double voltage = 1e6;  // 1 MV
    RFCavity cavity("TestCavity", 0.5, voltage, 400e6, 0.0);

    // At phase = 0, full acceleration
    double gain0 = cavity.getEnergyGain(0.0);
    EXPECT_NEAR(gain0, e * voltage, 1e-20);

    // At phase = pi/2, no acceleration
    double gain90 = cavity.getEnergyGain(physics::constants::pi / 2.0);
    EXPECT_NEAR(gain90, 0.0, 1e-10);
}

TEST_F(ComponentTest, RFCavityFieldSource) {
    RFCavity cavity("TestCavity", 0.5, 1e6, 400e6, 0.0);

    auto field = cavity.getFieldSource();
    EXPECT_NE(field, nullptr);
}

// Detector tests

TEST_F(ComponentTest, DetectorRecordsHits) {
    Detector detector("TestDetector");

    EXPECT_EQ(detector.getType(), ComponentType::Detector);
    EXPECT_EQ(detector.getHitCount(), 0u);

    detector.recordHit(1.0, glm::dvec3(0.0), glm::dvec3(1e-18, 0, 0), 42);
    EXPECT_EQ(detector.getHitCount(), 1u);

    const auto& hits = detector.getHits();
    EXPECT_EQ(hits[0].particleId, 42u);
}

TEST_F(ComponentTest, DetectorClearsHits) {
    Detector detector("TestDetector");

    detector.recordHit(1.0, glm::dvec3(0.0), glm::dvec3(1e-18, 0, 0), 1);
    detector.recordHit(2.0, glm::dvec3(0.0), glm::dvec3(1e-18, 0, 0), 2);
    EXPECT_EQ(detector.getHitCount(), 2u);

    detector.clearHits();
    EXPECT_EQ(detector.getHitCount(), 0u);
}

// Coordinate transformation tests

TEST_F(ComponentTest, LocalGlobalTransform) {
    BeamPipe pipe("TestPipe", 1.0);
    pipe.setPosition(glm::dvec3(1.0, 2.0, 3.0));

    glm::dvec3 global(1.5, 2.5, 3.5);
    glm::dvec3 local = pipe.toLocal(global);

    EXPECT_NEAR(local.x, 0.5, EPSILON);
    EXPECT_NEAR(local.y, 0.5, EPSILON);
    EXPECT_NEAR(local.z, 0.5, EPSILON);

    glm::dvec3 backToGlobal = pipe.toGlobal(local);
    EXPECT_NEAR(backToGlobal.x, global.x, EPSILON);
    EXPECT_NEAR(backToGlobal.y, global.y, EPSILON);
    EXPECT_NEAR(backToGlobal.z, global.z, EPSILON);
}

TEST_F(ComponentTest, SPositionTracking) {
    BeamPipe pipe("TestPipe", 2.0);

    pipe.setSPosition(5.0);

    EXPECT_DOUBLE_EQ(pipe.getSPosition(), 5.0);
    EXPECT_DOUBLE_EQ(pipe.getEntranceS(), 5.0);
    EXPECT_DOUBLE_EQ(pipe.getExitS(), 7.0);
}

TEST_F(ComponentTest, ContainsSCheck) {
    BeamPipe pipe("TestPipe", 2.0);
    pipe.setSPosition(5.0);

    EXPECT_TRUE(pipe.containsS(5.0));
    EXPECT_TRUE(pipe.containsS(6.0));
    EXPECT_FALSE(pipe.containsS(7.0));  // Exit is exclusive
    EXPECT_FALSE(pipe.containsS(4.9));
}

// Component type string conversion

TEST_F(ComponentTest, ComponentTypeToString) {
    EXPECT_EQ(componentTypeToString(ComponentType::BeamPipe), "BeamPipe");
    EXPECT_EQ(componentTypeToString(ComponentType::Dipole), "Dipole");
    EXPECT_EQ(componentTypeToString(ComponentType::Quadrupole), "Quadrupole");
    EXPECT_EQ(componentTypeToString(ComponentType::RFCavity), "RFCavity");
    EXPECT_EQ(componentTypeToString(ComponentType::Detector), "Detector");
}

} // namespace pas::accelerator::tests
