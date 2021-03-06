#include "VRPyEngineering.h"
#include "core/scripting/VRPyBaseT.h"
#include "core/scripting/VRPyTypeCaster.h"
#include "core/scripting/VRPyGeometry.h"
#include "core/scripting/VRPyPath.h"

using namespace OSG;

simpleVRPyType(NumberingEngine, New_VRObjects_ptr);
simpleVRPyType(RobotArm, New_named_ptr);
simpleVRPyType(PipeSystem, New_ptr);
simpleVRPyType(RocketExhaust, New_VRObjects_ptr);

PyMethodDef VRPyPipeSystem::methods[] = {
    {"addNode", PyWrap( PipeSystem, addNode, "Add node, type can be [Tank, Valve, Outlet, Pump]", int, string, PosePtr, string, map<string, string> ) },
    {"addSegment", PyWrap( PipeSystem, addSegment, "Add segment between nodes (radius, n1, n2)", int, double, int, int ) },
    {"setDoVisual", PyWrap( PipeSystem, setDoVisual, "Enable visual", void, bool ) },
    {"setNodePose", PyWrap( PipeSystem, setNodePose, "Set node pose by ID", void, int, PosePtr ) },
    {"disconnect", PyWrap( PipeSystem, disconnect, "Disconnect a node from a segment, keeps the segment by adding a junction to its end (nId, sID)", int, int, int ) },
    {"insertSegment", PyWrap( PipeSystem, insertSegment, "Insert a segment between a node and segment (nID, sID, radius)", int, int, int, float ) },
    {"getNode", PyWrap( PipeSystem, getNode, "Get node ID by name", int, string ) },
    {"getNodePose", PyWrap( PipeSystem, getNodePose, "Get node pose", PosePtr, int ) },
    {"getSegment", PyWrap( PipeSystem, getSegment, "Get segment ID by its node IDs", int, int, int ) },
    {"getSegmentPressure", PyWrap( PipeSystem, getSegmentPressure, "Get segment pressure", double, int ) },
    {"getSegmentFlow", PyWrap( PipeSystem, getSegmentFlow, "Get segment flow", double, int ) },
    {"getTankPressure", PyWrap( PipeSystem, getTankPressure, "Get tank pressure", double, string ) },
    {"getTankDensity", PyWrap( PipeSystem, getTankDensity, "Get tank density", double, string ) },
    {"getTankVolume", PyWrap( PipeSystem, getTankVolume, "Get tank volume", double, string ) },
    {"getPump", PyWrap( PipeSystem, getPump, "Get pump performance", double, string ) },
    {"setPump", PyWrap( PipeSystem, setPump, "Set pump performance", void, string, double ) },
    {"setValve", PyWrap( PipeSystem, setValve, "Set valve state", void, string, bool ) },
    {"setTankPressure", PyWrap( PipeSystem, setTankPressure, "Set tank pressure", void, string, double ) },
    {"setTankDensity", PyWrap( PipeSystem, setTankDensity, "Set tank density", void, string, double ) },
    {"printSystem", PyWrap( PipeSystem, printSystem, "Print system state to console", void ) },
    {NULL}
};

PyMethodDef VRPyNumberingEngine::methods[] = {
    {"set", PyWrapOpt( NumberingEngine, set, "Set number: ID, pos, number, decimal places, groupID", "2|0", void, int, Vec3d, float, int, int ) },
    {"clear", PyWrap( NumberingEngine, clear, "Clear numbers", void ) },
    {NULL}
};

PyMethodDef VRPyRobotArm::methods[] = {
    {"showAnalytics", PyWrap(RobotArm, showAnalytics, "Shows a visualization of the analytic model", void, bool ) },
    {"setParts", PyWrap(RobotArm, setParts, "Set robot parts", void, vector<VRTransformPtr> ) },
    {"setAngleOffsets", PyWrap(RobotArm, setAngleOffsets, "Set angle offset for each part", void, vector<float> ) },
    {"setAngleDirections", PyWrap(RobotArm, setAngleDirections, "Set angles rotation direction - setAngleDirections([1/-1])", void, vector<int> ) },
    {"setAxis", PyWrap(RobotArm, setAxis, "Set rotation axis for each part - setAxis([int a])\n a: 0 = 'x', 1 = 'y', 2 = 'z'", void, vector<int> ) },
    {"setLengths", PyWrap(RobotArm, setLengths, "Set kinematic lengths between joints - setLengths([base_height, upper_arm length, forearm length, grab position])", void, vector<float> ) },
    {"setMaxSpeed", PyWrap(RobotArm, setMaxSpeed, "Set max angular speed", void, float ) },
    {"moveTo", PyWrap(RobotArm, moveTo, "Move the end effector to a certain position - moveTo([x,y,z])", void, PosePtr ) },
    {"setGrab", PyWrap(RobotArm, setGrab, "Set grab state - setGrab(float d)\n d: 0 is closed, 1 is open", void, float ) },
    {"toggleGrab", PyWrap(RobotArm, toggleGrab, "Toggle the grab - toggleGrab()", void ) },
    {"grab", PyWrap(RobotArm, grab, "Grab object", void, VRTransformPtr ) },
    {"drop", PyWrap(RobotArm, drop, "Drop held object", void ) },
    {"setAngles", PyWrapOpt(RobotArm, setAngles, "Set joint angles - setAngles( angles, force )", "0", void, vector<float>, bool ) },
    {"getAngles", PyWrap(RobotArm, getAngles, "Get joint angles - getAngles()", vector<float> ) },
    //{"getForwardKinematics", PyWrap(RobotArm, getForwardKinematics, "Get end effector pose from angles - p,d,u getForwardKinematics( angles )") },
    //{"getBackwardKinematics", PyWrap(RobotArm, getBackwardKinematics, "Get angles from end effector pose - angles getBackwardKinematics( p,d,u )") },
    {"setPath", PyWrapOpt(RobotArm, setPath, "Set robot path(s) - second path is optinal and overrides orientation", "0", void, PathPtr, PathPtr ) },
    {"getPath", PyWrap(RobotArm, getPath, "Get robot path", PathPtr ) },
    {"getOrientationPath", PyWrap(RobotArm, getOrientationPath, "Get robot orientation path", PathPtr ) },
    {"getParts", PyWrap(RobotArm, getParts, "Get robot parts", vector<VRTransformPtr> ) },
    {"moveOnPath", PyWrapOpt(RobotArm, moveOnPath, "Move robot on internal path - moveOnPath(t0, t1, doLoop, durationMultiplier)", "0|1", void, float, float, bool, float) },
    {"isMoving", PyWrap(RobotArm, isMoving, "Get animation status", bool) },
    {"setEventCallback", PyWrap(RobotArm, setEventCallback, "Set callback for move and stop events", void, VRMessageCbPtr) },
    {NULL}  /* Sentinel */
};

PyMethodDef VRPyRocketExhaust::methods[] = {
    {"set", PyWrap( RocketExhaust, set, "Set exhaust amount, from 0.0 to 1.0", void, float ) },
    {NULL}
};
