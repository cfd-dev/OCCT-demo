#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <gp_Sphere.hxx>
#include <Geom_SphericalSurface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Pnt.hxx>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <tbb/parallel_for.h>
#include <tbb/task_arena.h>
#include <tbb/enumerable_thread_specific.h>

// 随机生成不在球面上的三维点
void generate_random_points(std::vector<gp_Pnt> &points, int num_points, double min, double max, double sphere_radius)
{
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(min, max);
    for (int i = 0; i < num_points; ++i)
    {
        double x, y, z;
        do
        {
            x = dist(rng);
            y = dist(rng);
            z = dist(rng);
        } while (std::sqrt(x * x + y * y + z * z) < sphere_radius * 0.95); // 保证点不在球面上
        points.emplace_back(x, y, z);
    }
}

// 串行投影（每次只构造一次projector，效率高）
void project_points_serial_fast(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere)
{
    GeomAPI_ProjectPointOnSurf projector; // 只构造一次 projector，减少构造/析构开销，提高效率
    for (size_t i = 0; i < points.size(); ++i)
    {
        projector.Init(points[i], sphere);
        projected[i] = projector.NearestPoint();
    }
}

// OpenMP并行投影（每线程只构造一次projector，效率高）
void project_points_omp_fast(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere, int num_threads)
{
#ifdef _OPENMP
    omp_set_num_threads(num_threads);
#pragma omp parallel
    {
        GeomAPI_ProjectPointOnSurf projector; // 每个线程只构造一次 projector，减少构造/析构开销，提高效率
#pragma omp for
        for (int i = 0; i < static_cast<int>(points.size()); ++i)
        {
            projector.Init(points[i], sphere);
            projected[i] = projector.NearestPoint();
        }
    }
#else
    std::cerr << "OpenMP not enabled!" << std::endl;
#endif
}

// TBB并行投影（每线程只构造一次projector，效率高）
void project_points_tbb_fast(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere, int num_threads)
{
    tbb::enumerable_thread_specific<GeomAPI_ProjectPointOnSurf> ets_projector;
    tbb::task_arena arena(num_threads);
    arena.execute([&]
                  { tbb::parallel_for(size_t(0), points.size(), [&](size_t i)
                                      {
            auto& projector = ets_projector.local(); // 每个线程只构造一次 projector，减少构造/析构开销，提高效率
            projector.Init(points[i], sphere);
            projected[i] = projector.NearestPoint(); }); });
}

// 串行投影（每次都构造新的projector，效率低，仅作对比）
void project_points_serial(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        GeomAPI_ProjectPointOnSurf projector(points[i], sphere);
        projected[i] = projector.NearestPoint();
    }
}

// OpenMP并行投影（每次都构造新的projector，效率低，仅作对比）
void project_points_omp(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere, int num_threads)
{
#ifdef _OPENMP
    omp_set_num_threads(num_threads);
#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(points.size()); ++i)
    {
        GeomAPI_ProjectPointOnSurf projector(points[i], sphere);
        projected[i] = projector.NearestPoint();
    }
#else
    std::cerr << "OpenMP not enabled!" << std::endl;
#endif
}

// TBB并行投影（每次都构造新的projector，效率低，仅作对比）
void project_points_tbb(const std::vector<gp_Pnt> &points, 
                        std::vector<gp_Pnt> &projected, 
                        const Handle(Geom_SphericalSurface) & sphere, 
                        int num_threads)
{
    tbb::task_arena arena(num_threads);
    arena.execute([&] { 
        tbb::parallel_for(size_t(0), points.size(), [&](size_t i) {
            GeomAPI_ProjectPointOnSurf projector(points[i], sphere);
            projected[i] = projector.NearestPoint(); 
        }); 
    });
}

int main()
{
    int num_points = 8000000; // 点的数量，可调整
    int num_threads = 12;     // 线程数，可调整
    
    // 创建球面
    double sphere_radius = 50.0;
    gp_Ax3 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    gp_Sphere sphere(axis, sphere_radius);
    Handle(Geom_SphericalSurface) geomSphere = new Geom_SphericalSurface(sphere);

    // 生成随机点
    std::vector<gp_Pnt> points;
    points.reserve(num_points);
    generate_random_points(points, num_points, -100.0, 100.0, sphere_radius);

    // 结果存储
    std::vector<gp_Pnt> projected_tbb(num_points);
    std::vector<gp_Pnt> projected_omp(num_points);
    std::vector<gp_Pnt> projected_serial(num_points);

    // 串行投影
    auto t1 = std::chrono::high_resolution_clock::now();
    project_points_serial_fast(points, projected_serial, geomSphere);
    auto t2 = std::chrono::high_resolution_clock::now();
    double serial_time = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "串行投影耗时: " << serial_time << " 秒" << std::endl;

    // TBB并行投影
    t1 = std::chrono::high_resolution_clock::now();
    project_points_tbb_fast(points, projected_tbb, geomSphere, num_threads);
    t2 = std::chrono::high_resolution_clock::now();
    double tbb_time = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "TBB并行投影耗时: " << tbb_time << " 秒" << std::endl;

    // OpenMP并行投影
    t1 = std::chrono::high_resolution_clock::now();
    project_points_omp_fast(points, projected_omp, geomSphere, num_threads);
    t2 = std::chrono::high_resolution_clock::now();
    double omp_time = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "OpenMP并行投影耗时: " << omp_time << " 秒" << std::endl;

    // 检查TBB投影点是否在球面上（根据球的解析表达式）
    int tbb_not_on_sphere = 0;
    for (const auto &pt : projected_tbb)
    {
        double dist = std::sqrt(pt.X() * pt.X() + pt.Y() * pt.Y() + pt.Z() * pt.Z());
        if (std::abs(dist - sphere_radius) > 1e-8)
        {
            ++tbb_not_on_sphere;
        }
    }
    std::cout << "TBB投影点不在球面上的数量: " << tbb_not_on_sphere << std::endl;

    // 检查OpenMP投影点是否在球面上
    int omp_not_on_sphere = 0;
    for (const auto &pt : projected_omp)
    {
        double dist = std::sqrt(pt.X() * pt.X() + pt.Y() * pt.Y() + pt.Z() * pt.Z());
        if (std::abs(dist - sphere_radius) > 1e-8)
        {
            ++omp_not_on_sphere;
        }
    }
    std::cout << "OpenMP投影点不在球面上的数量: " << omp_not_on_sphere << std::endl;

    // 验证并行投影与串行投影结果一致性
    int mismatch_tbb = 0, mismatch_omp = 0;
    for (int i = 0; i < num_points; ++i)
    {
        if (projected_serial[i].Distance(projected_tbb[i]) > 1e-8)
            ++mismatch_tbb;
        if (projected_serial[i].Distance(projected_omp[i]) > 1e-8)
            ++mismatch_omp;
    }
    std::cout << "串行与TBB投影结果不一致点数: " << mismatch_tbb << std::endl;
    std::cout << "串行与OpenMP投影结果不一致点数: " << mismatch_omp << std::endl;

    // 计算加速比和并行效率
    double tbb_speedup = serial_time / tbb_time;
    double omp_speedup = serial_time / omp_time;
    double tbb_efficiency = tbb_speedup / num_threads * 100.0;
    double omp_efficiency = omp_speedup / num_threads * 100.0;
    
    // 输出性能对比表格
    std::cout << "\n=================== 并行投影性能对比 ==============" << std::endl;
    std::cout << "方式      | 线程数 |   耗时(s) |  加速比 | 并行效率(%)" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;
    std::cout << "串行    | "
              << std::setw(6) << 1 << " | "
              << std::fixed << std::setprecision(3) << std::setw(8) << serial_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << 1.0 << " | "
              << std::setw(10) << 100.0 << std::endl;
    std::cout << "TBB     | "
              << std::setw(6) << num_threads << " | "
              << std::fixed << std::setprecision(3) << std::setw(8) << tbb_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << tbb_speedup << " | "
              << std::setw(10) << std::fixed << std::setprecision(2) << tbb_efficiency << std::endl;
    std::cout << "OpenMP  | "
              << std::setw(6) << num_threads << " | "
              << std::fixed << std::setprecision(3) << std::setw(8) << omp_time << " | "
              << std::fixed << std::setprecision(2) << std::setw(7) << omp_speedup << " | "
              << std::setw(10) << std::fixed << std::setprecision(2) << omp_efficiency << std::endl;
    std::cout << "====================================================" << std::endl;

    return 0;
}