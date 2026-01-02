#include <gtest/gtest.h>
#include <cmath>

#include "physics/EMField.hpp"
#include "physics/Constants.hpp"

namespace pas::physics::tests {

using namespace constants;

class EMFieldTest : public ::testing::Test {
protected:
    static constexpr double EPSILON = 1e-10;
};

// UniformBField tests

TEST_F(EMFieldTest, UniformBFieldReturnsConstantField) {
    glm::dvec3 B(0.0, 0.0, 1.5);  // 1.5 Tesla in z-direction
    UniformBField field(B);

    FieldValue value = field.evaluate(glm::dvec3(0.0), 0.0);

    EXPECT_DOUBLE_EQ(value.B.x, 0.0);
    EXPECT_DOUBLE_EQ(value.B.y, 0.0);
    EXPECT_DOUBLE_EQ(value.B.z, 1.5);
    EXPECT_DOUBLE_EQ(value.E.x, 0.0);
    EXPECT_DOUBLE_EQ(value.E.y, 0.0);
    EXPECT_DOUBLE_EQ(value.E.z, 0.0);
}

TEST_F(EMFieldTest, UniformBFieldIsTimeIndependent) {
    glm::dvec3 B(1.0, 2.0, 3.0);
    UniformBField field(B);

    FieldValue v1 = field.evaluate(glm::dvec3(0.0), 0.0);
    FieldValue v2 = field.evaluate(glm::dvec3(0.0), 1.0);
    FieldValue v3 = field.evaluate(glm::dvec3(0.0), 1000.0);

    EXPECT_DOUBLE_EQ(v1.B.x, v2.B.x);
    EXPECT_DOUBLE_EQ(v1.B.y, v2.B.y);
    EXPECT_DOUBLE_EQ(v1.B.z, v2.B.z);
    EXPECT_DOUBLE_EQ(v2.B.x, v3.B.x);
}

TEST_F(EMFieldTest, UniformBFieldWithBoundsRespectsRegion) {
    glm::dvec3 B(0.0, 1.0, 0.0);
    BoundingBox bounds(glm::dvec3(-1.0), glm::dvec3(1.0));
    UniformBField field(B, bounds);

    // Inside bounds
    FieldValue inside = field.evaluate(glm::dvec3(0.0), 0.0);
    EXPECT_DOUBLE_EQ(inside.B.y, 1.0);

    // Outside bounds
    FieldValue outside = field.evaluate(glm::dvec3(2.0, 0.0, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(outside.B.y, 0.0);
}

// QuadrupoleField tests

TEST_F(EMFieldTest, QuadrupoleFieldIsZeroAtCenter) {
    QuadrupoleField field(100.0);  // 100 T/m gradient

    FieldValue value = field.evaluate(glm::dvec3(0.0), 0.0);

    EXPECT_DOUBLE_EQ(value.B.x, 0.0);
    EXPECT_DOUBLE_EQ(value.B.y, 0.0);
    EXPECT_DOUBLE_EQ(value.B.z, 0.0);
}

TEST_F(EMFieldTest, QuadrupoleFieldLinearGradient) {
    double gradient = 50.0;  // 50 T/m
    QuadrupoleField field(gradient);

    // At (x=0.01, y=0)
    FieldValue v1 = field.evaluate(glm::dvec3(0.01, 0.0, 0.0), 0.0);
    EXPECT_NEAR(v1.B.x, 0.0, EPSILON);
    EXPECT_NEAR(v1.B.y, gradient * 0.01, EPSILON);  // By = G * x

    // At (x=0, y=0.01)
    FieldValue v2 = field.evaluate(glm::dvec3(0.0, 0.01, 0.0), 0.0);
    EXPECT_NEAR(v2.B.x, gradient * 0.01, EPSILON);  // Bx = G * y
    EXPECT_NEAR(v2.B.y, 0.0, EPSILON);
}

TEST_F(EMFieldTest, QuadrupoleFieldOutsideApertureIsZero) {
    double gradient = 100.0;
    double aperture = 0.05;  // 5 cm aperture
    QuadrupoleField field(gradient, glm::dvec3(0.0), 1.0, aperture);

    // Outside aperture radially
    FieldValue outside = field.evaluate(glm::dvec3(0.1, 0.0, 0.0), 0.0);
    EXPECT_DOUBLE_EQ(outside.B.x, 0.0);
    EXPECT_DOUBLE_EQ(outside.B.y, 0.0);
}

// RFField tests

TEST_F(EMFieldTest, RFFieldHasCorrectAmplitude) {
    double voltage = 1e6;  // 1 MV
    double length = 0.5;   // 0.5 m
    double frequency = 400e6;  // 400 MHz

    RFField field(voltage, frequency, 0.0, glm::dvec3(0.0), length, 0.1);

    // At t=0, phase=0: E = V/L * cos(0) = V/L
    FieldValue value = field.evaluate(glm::dvec3(0.0), 0.0);
    double expectedE = voltage / length;

    EXPECT_NEAR(value.E.z, expectedE, expectedE * 1e-10);
    EXPECT_DOUBLE_EQ(value.E.x, 0.0);
    EXPECT_DOUBLE_EQ(value.E.y, 0.0);
}

TEST_F(EMFieldTest, RFFieldOscillates) {
    double voltage = 1e6;
    double frequency = 1e9;  // 1 GHz
    RFField field(voltage, frequency, 0.0, glm::dvec3(0.0), 0.5, 0.1);

    // At t = 0
    FieldValue v0 = field.evaluate(glm::dvec3(0.0), 0.0);

    // At t = period/4 (quarter period)
    double period = 1.0 / frequency;
    FieldValue v1 = field.evaluate(glm::dvec3(0.0), period / 4.0);

    // At quarter period, cos(pi/2) = 0
    EXPECT_NEAR(v1.E.z, 0.0, 1e-6);

    // Original should be maximum
    EXPECT_NEAR(v0.E.z, voltage / 0.5, 1e-6);
}

TEST_F(EMFieldTest, RFFieldPhaseOffset) {
    double voltage = 1e6;
    double frequency = 1e9;
    double phase = constants::pi / 2.0;  // 90 degree phase
    RFField field(voltage, frequency, phase, glm::dvec3(0.0), 0.5, 0.1);

    // At t=0 with 90 deg phase: E = V/L * cos(pi/2) = 0
    FieldValue value = field.evaluate(glm::dvec3(0.0), 0.0);
    EXPECT_NEAR(value.E.z, 0.0, 1e-6);
}

// EMFieldManager tests

TEST_F(EMFieldTest, ManagerWithNoSourcesReturnsZero) {
    EMFieldManager manager;

    FieldValue value = manager.evaluate(glm::dvec3(0.0), 0.0);

    EXPECT_DOUBLE_EQ(value.E.x, 0.0);
    EXPECT_DOUBLE_EQ(value.B.x, 0.0);
}

TEST_F(EMFieldTest, ManagerSumsMultipleSources) {
    EMFieldManager manager;

    auto field1 = std::make_shared<UniformBField>(glm::dvec3(1.0, 0.0, 0.0));
    auto field2 = std::make_shared<UniformBField>(glm::dvec3(0.0, 2.0, 0.0));

    manager.addSource(field1);
    manager.addSource(field2);

    FieldValue value = manager.evaluate(glm::dvec3(0.0), 0.0);

    EXPECT_DOUBLE_EQ(value.B.x, 1.0);
    EXPECT_DOUBLE_EQ(value.B.y, 2.0);
    EXPECT_DOUBLE_EQ(value.B.z, 0.0);
}

TEST_F(EMFieldTest, DisabledSourceIsIgnored) {
    EMFieldManager manager;

    auto field = std::make_shared<UniformBField>(glm::dvec3(0.0, 0.0, 5.0));
    field->setEnabled(false);
    manager.addSource(field);

    FieldValue value = manager.evaluate(glm::dvec3(0.0), 0.0);

    EXPECT_DOUBLE_EQ(value.B.z, 0.0);
}

TEST_F(EMFieldTest, ManagerClearRemovesAllSources) {
    EMFieldManager manager;

    manager.addSource(std::make_shared<UniformBField>(glm::dvec3(1.0, 0.0, 0.0)));
    EXPECT_EQ(manager.getSourceCount(), 1u);

    manager.clear();
    EXPECT_EQ(manager.getSourceCount(), 0u);
}

// FieldValue operations

TEST_F(EMFieldTest, FieldValueAddition) {
    FieldValue f1(glm::dvec3(1.0, 0.0, 0.0), glm::dvec3(0.0, 1.0, 0.0));
    FieldValue f2(glm::dvec3(0.0, 2.0, 0.0), glm::dvec3(0.0, 0.0, 3.0));

    FieldValue sum = f1 + f2;

    EXPECT_DOUBLE_EQ(sum.E.x, 1.0);
    EXPECT_DOUBLE_EQ(sum.E.y, 2.0);
    EXPECT_DOUBLE_EQ(sum.B.y, 1.0);
    EXPECT_DOUBLE_EQ(sum.B.z, 3.0);
}

TEST_F(EMFieldTest, FieldValueScalarMultiplication) {
    FieldValue f(glm::dvec3(1.0, 2.0, 3.0), glm::dvec3(4.0, 5.0, 6.0));

    FieldValue scaled = f * 2.0;

    EXPECT_DOUBLE_EQ(scaled.E.x, 2.0);
    EXPECT_DOUBLE_EQ(scaled.E.y, 4.0);
    EXPECT_DOUBLE_EQ(scaled.B.z, 12.0);
}

// BoundingBox tests

TEST_F(EMFieldTest, BoundingBoxContainsPoint) {
    BoundingBox box(glm::dvec3(-1.0), glm::dvec3(1.0));

    EXPECT_TRUE(box.contains(glm::dvec3(0.0)));
    EXPECT_TRUE(box.contains(glm::dvec3(0.5, 0.5, 0.5)));
    EXPECT_TRUE(box.contains(glm::dvec3(-1.0, -1.0, -1.0)));  // Edge
    EXPECT_FALSE(box.contains(glm::dvec3(1.1, 0.0, 0.0)));
}

TEST_F(EMFieldTest, DefaultBoundingBoxIsInfinite) {
    BoundingBox box;

    EXPECT_TRUE(box.isInfinite());
    EXPECT_TRUE(box.contains(glm::dvec3(1e10, 1e10, 1e10)));
}

} // namespace pas::physics::tests
