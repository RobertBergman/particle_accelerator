#include <gtest/gtest.h>
#include <cmath>

#include "accelerator/Accelerator.hpp"
#include "physics/Constants.hpp"

namespace pas::accelerator::tests {

using namespace physics::constants;

class AcceleratorTest : public ::testing::Test {
protected:
    Accelerator accelerator;
    static constexpr double EPSILON = 1e-10;
};

// Basic operations

TEST_F(AcceleratorTest, InitiallyEmpty) {
    EXPECT_EQ(accelerator.getComponentCount(), 0u);
    EXPECT_DOUBLE_EQ(accelerator.getTotalLength(), 0.0);
}

TEST_F(AcceleratorTest, AddComponent) {
    auto pipe = std::make_shared<BeamPipe>("Pipe1", 1.0);
    accelerator.addComponent(pipe);

    EXPECT_EQ(accelerator.getComponentCount(), 1u);
}

TEST_F(AcceleratorTest, AddMultipleComponents) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe3", 3.0));

    EXPECT_EQ(accelerator.getComponentCount(), 3u);
}

TEST_F(AcceleratorTest, GetComponentByIndex) {
    auto pipe = std::make_shared<BeamPipe>("TestPipe", 1.0);
    accelerator.addComponent(pipe);

    auto retrieved = accelerator.getComponent(0);
    EXPECT_EQ(retrieved->getName(), "TestPipe");
}

TEST_F(AcceleratorTest, GetComponentByName) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));

    auto pipe2 = accelerator.getComponent("Pipe2");
    EXPECT_NE(pipe2, nullptr);
    EXPECT_EQ(pipe2->getName(), "Pipe2");
}

TEST_F(AcceleratorTest, RemoveComponentByIndex) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));

    accelerator.removeComponent(0);

    EXPECT_EQ(accelerator.getComponentCount(), 1u);
    EXPECT_EQ(accelerator.getComponent(0)->getName(), "Pipe2");
}

TEST_F(AcceleratorTest, RemoveComponentByName) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));

    accelerator.removeComponent("Pipe1");

    EXPECT_EQ(accelerator.getComponentCount(), 1u);
    EXPECT_EQ(accelerator.getComponent(0)->getName(), "Pipe2");
}

TEST_F(AcceleratorTest, Clear) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));

    accelerator.clear();

    EXPECT_EQ(accelerator.getComponentCount(), 0u);
}

// Lattice computation

TEST_F(AcceleratorTest, ComputeLatticeCalculatesTotalLength) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe3", 3.0));

    accelerator.computeLattice();

    EXPECT_DOUBLE_EQ(accelerator.getTotalLength(), 6.0);
}

TEST_F(AcceleratorTest, ComputeLatticeSetsSPositions) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe3", 3.0));

    accelerator.computeLattice();

    EXPECT_DOUBLE_EQ(accelerator.getComponent(0)->getSPosition(), 0.0);
    EXPECT_DOUBLE_EQ(accelerator.getComponent(1)->getSPosition(), 1.0);
    EXPECT_DOUBLE_EQ(accelerator.getComponent(2)->getSPosition(), 3.0);
}

TEST_F(AcceleratorTest, GetComponentAtS) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe1", 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe2", 2.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe3", 3.0));

    accelerator.computeLattice();

    auto atS0 = accelerator.getComponentAtS(0.5);
    EXPECT_EQ(atS0->getName(), "Pipe1");

    auto atS2 = accelerator.getComponentAtS(2.0);
    EXPECT_EQ(atS2->getName(), "Pipe2");

    auto atS5 = accelerator.getComponentAtS(5.0);
    EXPECT_EQ(atS5->getName(), "Pipe3");
}

// FODO cell construction

TEST_F(AcceleratorTest, BuildFODOCell) {
    FODOCellParams params;
    params.cellLength = 10.0;
    params.quadLength = 0.5;
    params.quadGradient = 50.0;

    accelerator.buildFODOCell(params, "TestCell");

    // FODO cell has: QF, Drift, QD, Drift = 4 components
    EXPECT_EQ(accelerator.getComponentCount(), 4u);
    EXPECT_EQ(accelerator.getQuadrupoleCount(), 2u);
}

TEST_F(AcceleratorTest, FODOCellHasFocusingAndDefocusing) {
    FODOCellParams params;
    params.quadGradient = 50.0;

    accelerator.buildFODOCell(params);

    auto quads = accelerator.getQuadrupoles();
    EXPECT_EQ(quads.size(), 2u);

    // First should be focusing, second defocusing
    EXPECT_TRUE(quads[0]->isFocusing());
    EXPECT_FALSE(quads[1]->isFocusing());
}

TEST_F(AcceleratorTest, BuildFODOLattice) {
    FODOCellParams params;
    params.cellLength = 5.0;

    accelerator.buildFODOLattice(params, 4);

    // 4 cells * 4 components = 16 components
    EXPECT_EQ(accelerator.getComponentCount(), 16u);
    EXPECT_EQ(accelerator.getQuadrupoleCount(), 8u);
}

// Lattice type

TEST_F(AcceleratorTest, DefaultIsLinear) {
    EXPECT_EQ(accelerator.getLatticeType(), LatticeType::Linear);
    EXPECT_FALSE(accelerator.isClosed());
}

TEST_F(AcceleratorTest, CloseRing) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe", 100.0));
    accelerator.closeRing();

    EXPECT_EQ(accelerator.getLatticeType(), LatticeType::Circular);
    EXPECT_TRUE(accelerator.isClosed());
}

TEST_F(AcceleratorTest, CircumferenceEqualsLength) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Pipe", 100.0));
    accelerator.computeLattice();

    EXPECT_DOUBLE_EQ(accelerator.getCircumference(), 100.0);
}

// Field population

TEST_F(AcceleratorTest, PopulateFieldManager) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Drift", 1.0));
    accelerator.addComponent(std::make_shared<Dipole>("Dipole", 1.0, 1.0));
    accelerator.addComponent(std::make_shared<Quadrupole>("Quad", 0.5, 50.0));

    physics::EMFieldManager manager;
    accelerator.populateFieldManager(manager);

    // BeamPipe has no field, Dipole and Quad do
    EXPECT_EQ(manager.getSourceCount(), 2u);
}

// Component filtering

TEST_F(AcceleratorTest, GetDipoles) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Drift", 1.0));
    accelerator.addComponent(std::make_shared<Dipole>("D1", 1.0, 1.0));
    accelerator.addComponent(std::make_shared<Dipole>("D2", 1.0, 1.5));
    accelerator.addComponent(std::make_shared<Quadrupole>("Q1", 0.5, 50.0));

    auto dipoles = accelerator.getDipoles();
    EXPECT_EQ(dipoles.size(), 2u);
}

TEST_F(AcceleratorTest, GetQuadrupoles) {
    accelerator.addComponent(std::make_shared<Quadrupole>("Q1", 0.5, 50.0));
    accelerator.addComponent(std::make_shared<Dipole>("D1", 1.0, 1.0));
    accelerator.addComponent(std::make_shared<Quadrupole>("Q2", 0.5, -50.0));

    auto quads = accelerator.getQuadrupoles();
    EXPECT_EQ(quads.size(), 2u);
}

TEST_F(AcceleratorTest, GetRFCavities) {
    accelerator.addComponent(std::make_shared<BeamPipe>("Drift", 1.0));
    accelerator.addComponent(std::make_shared<RFCavity>("RF1", 0.5, 1e6, 400e6));
    accelerator.addComponent(std::make_shared<RFCavity>("RF2", 0.5, 2e6, 400e6));

    auto cavities = accelerator.getRFCavities();
    EXPECT_EQ(cavities.size(), 2u);
}

// Counts

TEST_F(AcceleratorTest, DipoleCount) {
    accelerator.addComponent(std::make_shared<Dipole>("D1", 1.0, 1.0));
    accelerator.addComponent(std::make_shared<Dipole>("D2", 1.0, 1.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Drift", 1.0));

    EXPECT_EQ(accelerator.getDipoleCount(), 2u);
}

TEST_F(AcceleratorTest, QuadrupoleCount) {
    accelerator.addComponent(std::make_shared<Quadrupole>("Q1", 0.5, 50.0));
    accelerator.addComponent(std::make_shared<Quadrupole>("Q2", 0.5, -50.0));
    accelerator.addComponent(std::make_shared<BeamPipe>("Drift", 1.0));

    EXPECT_EQ(accelerator.getQuadrupoleCount(), 2u);
}

// Total bending angle

TEST_F(AcceleratorTest, TotalBendingAngle) {
    double B1 = 1.0;
    double B2 = 1.5;
    double L = 1.0;

    accelerator.addComponent(std::make_shared<Dipole>("D1", L, B1));
    accelerator.addComponent(std::make_shared<Dipole>("D2", L, B2));

    double momentum = 1e-18;
    double totalAngle = accelerator.getTotalBendingAngle(momentum);

    double expected = e * (B1 + B2) * L / momentum;
    EXPECT_NEAR(totalAngle, expected, EPSILON);
}

// Add drift helper

TEST_F(AcceleratorTest, AddDrift) {
    accelerator.addDrift(2.5, "TestDrift");

    EXPECT_EQ(accelerator.getComponentCount(), 1u);

    auto component = accelerator.getComponent(0);
    EXPECT_EQ(component->getType(), ComponentType::BeamPipe);
    EXPECT_EQ(component->getName(), "TestDrift");
    EXPECT_DOUBLE_EQ(component->getLength(), 2.5);
}

TEST_F(AcceleratorTest, AddDriftAutoName) {
    accelerator.addDrift(1.0);
    accelerator.addDrift(2.0);

    EXPECT_EQ(accelerator.getComponent(0)->getName(), "Drift_1");
    EXPECT_EQ(accelerator.getComponent(1)->getName(), "Drift_2");
}

} // namespace pas::accelerator::tests
