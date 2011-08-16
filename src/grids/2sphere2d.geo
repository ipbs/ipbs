// Define setup variables
radius = 40;
distance = 100.0;
height = 100;
width = 500;
outer_refinement = 10;
inner_refinement = 1;
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

// Define surface
Line Loop(12) = {2, 3, 4, 8, 9, 6, 7, 10, 11, -5, 1};
Plane Surface(13) = {12};

// ========== Physical Groups (Boundary Conditions) ===========

// Outer boundaries, which are set to Dirichlet B.C.
// Physical Line(0) = {1, 2, 3};
// Lower boundaries, wich are set to Neumann B.C.
Physical Line(1) = {4, 6, 7, 5, 1, 2, 3};
// Spheres which are set to IPBS B.C.
Physical Line(2) = {8, 9};
Physical Line(3) = {10, 11};

// Finally we also need a physical surface
Physical Surface(14) = {13};
