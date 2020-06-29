#include "open3d/pipelines/mesh_deform/TriangleMeshDeform.h"
#include "tests/UnitTest.h"

namespace open3d {
namespace tests {

TEST(TriangleMeshDeform, DeformAsRigidAsPossible) {
    geometry::TriangleMesh mesh_in;
    geometry::TriangleMesh mesh_gt;
    mesh_in.vertices_ = {
            {0.000000, 0.000000, 0.000000}, {0.000000, 1.000000, 0.000000},
            {1.000000, 1.000000, 0.000000}, {1.000000, 0.000000, 0.000000},
            {0.500000, 0.500000, 0.000000}, {0.500000, 1.000000, 0.000000},
            {0.000000, 0.500000, 0.000000}, {0.500000, 0.000000, 0.000000},
            {1.000000, 0.500000, 0.000000}, {0.250000, 0.250000, 0.000000},
            {0.250000, 0.500000, 0.000000}, {0.000000, 0.250000, 0.000000},
            {0.750000, 0.750000, 0.000000}, {0.750000, 1.000000, 0.000000},
            {0.500000, 0.750000, 0.000000}, {0.250000, 1.000000, 0.000000},
            {0.000000, 0.750000, 0.000000}, {0.250000, 0.750000, 0.000000},
            {0.750000, 0.500000, 0.000000}, {1.000000, 0.750000, 0.000000},
            {0.250000, 0.000000, 0.000000}, {0.500000, 0.250000, 0.000000},
            {0.750000, 0.000000, 0.000000}, {1.000000, 0.250000, 0.000000},
            {0.750000, 0.250000, 0.000000}, {0.125000, 0.125000, 0.000000},
            {0.125000, 0.250000, 0.000000}, {0.000000, 0.125000, 0.000000},
            {0.375000, 0.375000, 0.000000}, {0.375000, 0.500000, 0.000000},
            {0.250000, 0.375000, 0.000000}, {0.125000, 0.500000, 0.000000},
            {0.000000, 0.375000, 0.000000}, {0.125000, 0.375000, 0.000000},
            {0.625000, 0.625000, 0.000000}, {0.625000, 0.750000, 0.000000},
            {0.500000, 0.625000, 0.000000}, {0.875000, 0.875000, 0.000000},
            {0.875000, 1.000000, 0.000000}, {0.750000, 0.875000, 0.000000},
            {0.625000, 1.000000, 0.000000}, {0.500000, 0.875000, 0.000000},
            {0.625000, 0.875000, 0.000000}, {0.375000, 1.000000, 0.000000},
            {0.250000, 0.875000, 0.000000}, {0.375000, 0.875000, 0.000000},
            {0.125000, 1.000000, 0.000000}, {0.000000, 0.875000, 0.000000},
            {0.125000, 0.875000, 0.000000}, {0.000000, 0.625000, 0.000000},
            {0.125000, 0.625000, 0.000000}, {0.125000, 0.750000, 0.000000},
            {0.375000, 0.625000, 0.000000}, {0.375000, 0.750000, 0.000000},
            {0.250000, 0.625000, 0.000000}, {0.875000, 0.750000, 0.000000},
            {1.000000, 0.875000, 0.000000}, {0.625000, 0.500000, 0.000000},
            {0.750000, 0.625000, 0.000000}, {0.875000, 0.500000, 0.000000},
            {1.000000, 0.625000, 0.000000}, {0.875000, 0.625000, 0.000000},
            {0.375000, 0.250000, 0.000000}, {0.500000, 0.375000, 0.000000},
            {0.125000, 0.000000, 0.000000}, {0.250000, 0.125000, 0.000000},
            {0.375000, 0.000000, 0.000000}, {0.500000, 0.125000, 0.000000},
            {0.375000, 0.125000, 0.000000}, {0.625000, 0.000000, 0.000000},
            {0.750000, 0.125000, 0.000000}, {0.625000, 0.125000, 0.000000},
            {0.875000, 0.000000, 0.000000}, {1.000000, 0.125000, 0.000000},
            {0.875000, 0.125000, 0.000000}, {1.000000, 0.375000, 0.000000},
            {0.875000, 0.375000, 0.000000}, {0.875000, 0.250000, 0.000000},
            {0.625000, 0.375000, 0.000000}, {0.625000, 0.250000, 0.000000},
            {0.750000, 0.375000, 0.000000}};

    mesh_in.triangles_ = {
            {0, 25, 27},  {25, 9, 26},  {26, 11, 27}, {25, 26, 27},
            {9, 28, 30},  {28, 4, 29},  {29, 10, 30}, {28, 29, 30},
            {10, 31, 33}, {31, 6, 32},  {32, 11, 33}, {31, 32, 33},
            {9, 30, 26},  {30, 10, 33}, {33, 11, 26}, {30, 33, 26},
            {4, 34, 36},  {34, 12, 35}, {35, 14, 36}, {34, 35, 36},
            {12, 37, 39}, {37, 2, 38},  {38, 13, 39}, {37, 38, 39},
            {13, 40, 42}, {40, 5, 41},  {41, 14, 42}, {40, 41, 42},
            {12, 39, 35}, {39, 13, 42}, {42, 14, 35}, {39, 42, 35},
            {5, 43, 45},  {43, 15, 44}, {44, 17, 45}, {43, 44, 45},
            {15, 46, 48}, {46, 1, 47},  {47, 16, 48}, {46, 47, 48},
            {16, 49, 51}, {49, 6, 50},  {50, 17, 51}, {49, 50, 51},
            {15, 48, 44}, {48, 16, 51}, {51, 17, 44}, {48, 51, 44},
            {4, 36, 29},  {36, 14, 52}, {52, 10, 29}, {36, 52, 29},
            {14, 41, 53}, {41, 5, 45},  {45, 17, 53}, {41, 45, 53},
            {17, 50, 54}, {50, 6, 31},  {31, 10, 54}, {50, 31, 54},
            {14, 53, 52}, {53, 17, 54}, {54, 10, 52}, {53, 54, 52},
            {2, 37, 56},  {37, 12, 55}, {55, 19, 56}, {37, 55, 56},
            {12, 34, 58}, {34, 4, 57},  {57, 18, 58}, {34, 57, 58},
            {18, 59, 61}, {59, 8, 60},  {60, 19, 61}, {59, 60, 61},
            {12, 58, 55}, {58, 18, 61}, {61, 19, 55}, {58, 61, 55},
            {4, 28, 63},  {28, 9, 62},  {62, 21, 63}, {28, 62, 63},
            {9, 25, 65},  {25, 0, 64},  {64, 20, 65}, {25, 64, 65},
            {20, 66, 68}, {66, 7, 67},  {67, 21, 68}, {66, 67, 68},
            {9, 65, 62},  {65, 20, 68}, {68, 21, 62}, {65, 68, 62},
            {7, 69, 71},  {69, 22, 70}, {70, 24, 71}, {69, 70, 71},
            {22, 72, 74}, {72, 3, 73},  {73, 23, 74}, {72, 73, 74},
            {23, 75, 77}, {75, 8, 76},  {76, 24, 77}, {75, 76, 77},
            {22, 74, 70}, {74, 23, 77}, {77, 24, 70}, {74, 77, 70},
            {4, 63, 57},  {63, 21, 78}, {78, 18, 57}, {63, 78, 57},
            {21, 67, 79}, {67, 7, 71},  {71, 24, 79}, {67, 71, 79},
            {24, 76, 80}, {76, 8, 59},  {59, 18, 80}, {76, 59, 80},
            {21, 79, 78}, {79, 24, 80}, {80, 18, 78}, {79, 80, 78}};
    mesh_gt.vertices_ = {
            {0.000000, 0.000000, 0.000000}, {0.000000, 1.000000, 0.000000},
            {1.000000, 1.000000, 0.000000}, {1.000000, 0.000000, 0.000000},
            {0.500000, 0.500000, 0.400000}, {0.500000, 1.000000, 0.000000},
            {0.000000, 0.500000, 0.000000}, {0.500000, 0.000000, 0.000000},
            {1.000000, 0.500000, 0.000000}, {0.250492, 0.250492, 0.040018},
            {0.248198, 0.500000, 0.102907}, {0.000000, 0.250000, 0.000000},
            {0.749508, 0.749508, 0.040018}, {0.750000, 1.000000, 0.000000},
            {0.500000, 0.751802, 0.102907}, {0.250000, 1.000000, 0.000000},
            {0.000000, 0.750000, 0.000000}, {0.250492, 0.749508, 0.040018},
            {0.751802, 0.500000, 0.102907}, {1.000000, 0.750000, 0.000000},
            {0.250000, 0.000000, 0.000000}, {0.500000, 0.248198, 0.102907},
            {0.750000, 0.000000, 0.000000}, {1.000000, 0.250000, 0.000000},
            {0.749508, 0.250492, 0.040018}, {0.125000, 0.125000, 0.000000},
            {0.125000, 0.250000, 0.000000}, {0.000000, 0.125000, 0.000000},
            {0.366700, 0.366700, 0.181470}, {0.361404, 0.500000, 0.242836},
            {0.248165, 0.374408, 0.083221}, {0.125000, 0.500000, 0.000000},
            {0.000000, 0.375000, 0.000000}, {0.125000, 0.375000, 0.000000},
            {0.633300, 0.633300, 0.181470}, {0.625592, 0.751835, 0.083221},
            {0.500000, 0.638596, 0.242836}, {0.875000, 0.875000, 0.000000},
            {0.875000, 1.000000, 0.000000}, {0.750000, 0.875000, 0.000000},
            {0.625000, 1.000000, 0.000000}, {0.500000, 0.875000, 0.000000},
            {0.625000, 0.875000, 0.000000}, {0.375000, 1.000000, 0.000000},
            {0.250000, 0.875000, 0.000000}, {0.375000, 0.875000, 0.000000},
            {0.125000, 1.000000, 0.000000}, {0.000000, 0.875000, 0.000000},
            {0.125000, 0.875000, 0.000000}, {0.000000, 0.625000, 0.000000},
            {0.125000, 0.625000, 0.000000}, {0.125000, 0.750000, 0.000000},
            {0.366700, 0.633300, 0.181470}, {0.374408, 0.751835, 0.083221},
            {0.248165, 0.625592, 0.083221}, {0.875000, 0.750000, 0.000000},
            {1.000000, 0.875000, 0.000000}, {0.638596, 0.500000, 0.242836},
            {0.751835, 0.625592, 0.083221}, {0.875000, 0.500000, 0.000000},
            {1.000000, 0.625000, 0.000000}, {0.875000, 0.625000, 0.000000},
            {0.374408, 0.248165, 0.083221}, {0.500000, 0.361404, 0.242836},
            {0.125000, 0.000000, 0.000000}, {0.250000, 0.125000, 0.000000},
            {0.375000, 0.000000, 0.000000}, {0.500000, 0.125000, 0.000000},
            {0.375000, 0.125000, 0.000000}, {0.625000, 0.000000, 0.000000},
            {0.750000, 0.125000, 0.000000}, {0.625000, 0.125000, 0.000000},
            {0.875000, 0.000000, 0.000000}, {1.000000, 0.125000, 0.000000},
            {0.875000, 0.125000, 0.000000}, {1.000000, 0.375000, 0.000000},
            {0.875000, 0.375000, 0.000000}, {0.875000, 0.250000, 0.000000},
            {0.633300, 0.366700, 0.181470}, {0.625592, 0.248165, 0.083221},
            {0.751835, 0.374408, 0.083221}};

    mesh_gt.triangles_ = {
            {0, 25, 27},  {25, 9, 26},  {26, 11, 27}, {25, 26, 27},
            {9, 28, 30},  {28, 4, 29},  {29, 10, 30}, {28, 29, 30},
            {10, 31, 33}, {31, 6, 32},  {32, 11, 33}, {31, 32, 33},
            {9, 30, 26},  {30, 10, 33}, {33, 11, 26}, {30, 33, 26},
            {4, 34, 36},  {34, 12, 35}, {35, 14, 36}, {34, 35, 36},
            {12, 37, 39}, {37, 2, 38},  {38, 13, 39}, {37, 38, 39},
            {13, 40, 42}, {40, 5, 41},  {41, 14, 42}, {40, 41, 42},
            {12, 39, 35}, {39, 13, 42}, {42, 14, 35}, {39, 42, 35},
            {5, 43, 45},  {43, 15, 44}, {44, 17, 45}, {43, 44, 45},
            {15, 46, 48}, {46, 1, 47},  {47, 16, 48}, {46, 47, 48},
            {16, 49, 51}, {49, 6, 50},  {50, 17, 51}, {49, 50, 51},
            {15, 48, 44}, {48, 16, 51}, {51, 17, 44}, {48, 51, 44},
            {4, 36, 29},  {36, 14, 52}, {52, 10, 29}, {36, 52, 29},
            {14, 41, 53}, {41, 5, 45},  {45, 17, 53}, {41, 45, 53},
            {17, 50, 54}, {50, 6, 31},  {31, 10, 54}, {50, 31, 54},
            {14, 53, 52}, {53, 17, 54}, {54, 10, 52}, {53, 54, 52},
            {2, 37, 56},  {37, 12, 55}, {55, 19, 56}, {37, 55, 56},
            {12, 34, 58}, {34, 4, 57},  {57, 18, 58}, {34, 57, 58},
            {18, 59, 61}, {59, 8, 60},  {60, 19, 61}, {59, 60, 61},
            {12, 58, 55}, {58, 18, 61}, {61, 19, 55}, {58, 61, 55},
            {4, 28, 63},  {28, 9, 62},  {62, 21, 63}, {28, 62, 63},
            {9, 25, 65},  {25, 0, 64},  {64, 20, 65}, {25, 64, 65},
            {20, 66, 68}, {66, 7, 67},  {67, 21, 68}, {66, 67, 68},
            {9, 65, 62},  {65, 20, 68}, {68, 21, 62}, {65, 68, 62},
            {7, 69, 71},  {69, 22, 70}, {70, 24, 71}, {69, 70, 71},
            {22, 72, 74}, {72, 3, 73},  {73, 23, 74}, {72, 73, 74},
            {23, 75, 77}, {75, 8, 76},  {76, 24, 77}, {75, 76, 77},
            {22, 74, 70}, {74, 23, 77}, {77, 24, 70}, {74, 77, 70},
            {4, 63, 57},  {63, 21, 78}, {78, 18, 57}, {63, 78, 57},
            {21, 67, 79}, {67, 7, 71},  {71, 24, 79}, {67, 71, 79},
            {24, 76, 80}, {76, 8, 59},  {59, 18, 80}, {76, 59, 80},
            {21, 79, 78}, {79, 24, 80}, {80, 18, 78}, {79, 80, 78}};

    std::vector<int> constraint_ids = {
            1,  46, 47, 48, 16, 51, 49, 50, 6,  31, 33, 32, 11, 26, 27,
            25, 0,  64, 65, 20, 66, 68, 67, 7,  69, 71, 70, 22, 72, 74,
            73, 3,  15, 44, 43, 45, 5,  41, 40, 42, 13, 39, 37, 38, 2,
            56, 55, 19, 61, 60, 59, 8,  76, 75, 77, 23, 4};
    std::vector<Eigen::Vector3d> constraint_pos = {
            {0.000000, 1.000000, 0.000000}, {0.125000, 1.000000, 0.000000},
            {0.000000, 0.875000, 0.000000}, {0.125000, 0.875000, 0.000000},
            {0.000000, 0.750000, 0.000000}, {0.125000, 0.750000, 0.000000},
            {0.000000, 0.625000, 0.000000}, {0.125000, 0.625000, 0.000000},
            {0.000000, 0.500000, 0.000000}, {0.125000, 0.500000, 0.000000},
            {0.125000, 0.375000, 0.000000}, {0.000000, 0.375000, 0.000000},
            {0.000000, 0.250000, 0.000000}, {0.125000, 0.250000, 0.000000},
            {0.000000, 0.125000, 0.000000}, {0.125000, 0.125000, 0.000000},
            {0.000000, 0.000000, 0.000000}, {0.125000, 0.000000, 0.000000},
            {0.250000, 0.125000, 0.000000}, {0.250000, 0.000000, 0.000000},
            {0.375000, 0.000000, 0.000000}, {0.375000, 0.125000, 0.000000},
            {0.500000, 0.125000, 0.000000}, {0.500000, 0.000000, 0.000000},
            {0.625000, 0.000000, 0.000000}, {0.625000, 0.125000, 0.000000},
            {0.750000, 0.125000, 0.000000}, {0.750000, 0.000000, 0.000000},
            {0.875000, 0.000000, 0.000000}, {0.875000, 0.125000, 0.000000},
            {1.000000, 0.125000, 0.000000}, {1.000000, 0.000000, 0.000000},
            {0.250000, 1.000000, 0.000000}, {0.250000, 0.875000, 0.000000},
            {0.375000, 1.000000, 0.000000}, {0.375000, 0.875000, 0.000000},
            {0.500000, 1.000000, 0.000000}, {0.500000, 0.875000, 0.000000},
            {0.625000, 1.000000, 0.000000}, {0.625000, 0.875000, 0.000000},
            {0.750000, 1.000000, 0.000000}, {0.750000, 0.875000, 0.000000},
            {0.875000, 0.875000, 0.000000}, {0.875000, 1.000000, 0.000000},
            {1.000000, 1.000000, 0.000000}, {1.000000, 0.875000, 0.000000},
            {0.875000, 0.750000, 0.000000}, {1.000000, 0.750000, 0.000000},
            {0.875000, 0.625000, 0.000000}, {1.000000, 0.625000, 0.000000},
            {0.875000, 0.500000, 0.000000}, {1.000000, 0.500000, 0.000000},
            {0.875000, 0.375000, 0.000000}, {1.000000, 0.375000, 0.000000},
            {0.875000, 0.250000, 0.000000}, {1.000000, 0.250000, 0.000000},
            {0.500000, 0.500000, 0.400000}};

    auto mesh_deform = pipelines::mesh_deform::DeformAsRigidAsPossible(
            mesh_in, constraint_ids, constraint_pos, 50);

    double threshold = 1e-5;
    ExpectEQ(mesh_deform->vertices_, mesh_gt.vertices_, threshold);
    ExpectEQ(mesh_deform->vertex_normals_, mesh_gt.vertex_normals_, threshold);
    ExpectEQ(mesh_deform->vertex_colors_, mesh_gt.vertex_colors_, threshold);
    ExpectEQ(mesh_deform->triangles_, mesh_gt.triangles_);
    ExpectEQ(mesh_deform->triangle_normals_, mesh_gt.triangle_normals_,
             threshold);
}

}  // namespace tests
}  // namespace open3d
