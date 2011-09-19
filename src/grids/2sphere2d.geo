// Define setup variables
radius = 40;
distance = 100.0;
height = 100;
width = 300;
outer_refinement = 15;
inner_refinement = 0.5;
center_refinement = 5;

// ========== Setuo Geometry ===========

// Define points
Point(0) = {0, 0, 0, center_refinement}; // origin
Point(1) = {distance/2, 0, 0, 1.0}; // center of right sphere
Point(2) = {-distance/2, 0, 0, 1.0}; // center of left sphere
Point(3) = {-width/2, 0, 0, outer_refinement}; // lower left world coordinate
Point(4) = {width/2, 0, 0, outer_refinement}; // lower right world coordinate
Point(5) = {-width/2, height, 0, outer_refinement}; // upper left world coordinate
Point(6) = {width/2, height, 0, outer_refinement}; // upper right world coordinate
Point(7) = {distance/2+radius, 0, 0, inner_refinement}; // right boarder of right sphere
Point(8) = {distance/2-radius, 0, 0, inner_refinement}; // left boarder of right sphere
Point(9) = {distance/2, radius, 0, inner_refinement}; // top boarder of right sphere
Point(10) = {-distance/2+radius, 0, 0, inner_refinement}; // right boarder of left sphere
Point(11) = {-distance/2-radius, 0, 0, inner_refinement}; // left boarder of left sphere
Point(12) = {-distance/2, radius, 0, inner_refinement}; // top boarder of left sphere
Point(13) = {0, height, 0, 1}; // top boarder of left sphere

// Define lines
Line(1) = {3, 5}; // left boerder
Line(2) = {5, 6}; // upper boarder
Line(3) = {6, 4}; // left boerder
Line(4) = {4, 7}; // lower right boarder
Line(5) = {3, 11}; // lower left boarder
Line(6) = {8, 0}; //right middle lower boarder
Line(7) = {0, 10}; // left middle lower boarder
Circle(8) = {7, 1, 9}; // right sphere (right part)
Circle(9) = {9, 1, 8}; // right sphere (left part)
Circle(10) = {10, 2, 12}; // left sphere (right part)
Circle(11) = {12, 2, 11}; // left sphere (left part)
Line(15) = {0, 13};

// Define surface
Plane Surface(16) = {12};

// ========== Physical Groups (Boundary Conditions) ===========

// Outer boundaries, which are set to Dirichlet B.C.
// Physical Line(0) = {1, 2, 3};
// Lower boundaries, wich are set to Neumann B.C.
// Spheres which are set to IPBS B.C.

// Finally we also need a physical surface
