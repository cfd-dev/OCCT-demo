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

// ������ɲ��������ϵ���ά��
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
        } while (std::sqrt(x * x + y * y + z * z) < sphere_radius * 0.95); // ��֤�㲻��������
        points.emplace_back(x, y, z);
    }
}

// ����ͶӰ��ÿ��ֻ����һ��projector��Ч�ʸߣ�
void project_points_serial_fast(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere)
{
    GeomAPI_ProjectPointOnSurf projector; // ֻ����һ�� projector�����ٹ���/�������������Ч��
    for (size_t i = 0; i < points.size(); ++i)
    {
        projector.Init(points[i], sphere);
        projected[i] = projector.NearestPoint();
    }
}

// OpenMP����ͶӰ��ÿ�߳�ֻ����һ��projector��Ч�ʸߣ�
void project_points_omp_fast(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere, int num_threads)
{
#ifdef _OPENMP
    omp_set_num_threads(num_threads);
#pragma omp parallel
    {
        GeomAPI_ProjectPointOnSurf projector; // ÿ���߳�ֻ����һ�� projector�����ٹ���/�������������Ч��
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

// TBB����ͶӰ��ÿ�߳�ֻ����һ��projector��Ч�ʸߣ�
void project_points_tbb_fast(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere, int num_threads)
{
    tbb::enumerable_thread_specific<GeomAPI_ProjectPointOnSurf> ets_projector;
    tbb::task_arena arena(num_threads);
    arena.execute([&]
                  { tbb::parallel_for(size_t(0), points.size(), [&](size_t i)
                                      {
            auto& projector = ets_projector.local(); // ÿ���߳�ֻ����һ�� projector�����ٹ���/�������������Ч��
            projector.Init(points[i], sphere);
            projected[i] = projector.NearestPoint(); }); });
}

// ����ͶӰ��ÿ�ζ������µ�projector��Ч�ʵͣ������Աȣ�
void project_points_serial(const std::vector<gp_Pnt> &points, std::vector<gp_Pnt> &projected, const Handle(Geom_SphericalSurface) & sphere)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        GeomAPI_ProjectPointOnSurf projector(points[i], sphere);
        projected[i] = projector.NearestPoint();
    }
}

// OpenMP����ͶӰ��ÿ�ζ������µ�projector��Ч�ʵͣ������Աȣ�
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

// TBB����ͶӰ��ÿ�ζ������µ�projector��Ч�ʵͣ������Աȣ�
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
    int num_points = 8000000; // ����������ɵ���
    int num_threads = 12;     // �߳������ɵ���
    
    // ��������
    double sphere_radius = 50.0;
    gp_Ax3 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    gp_Sphere sphere(axis, sphere_radius);
    Handle(Geom_SphericalSurface) geomSphere = new Geom_SphericalSurface(sphere);

    // ���������
    std::vector<gp_Pnt> points;
    points.reserve(num_points);
    generate_random_points(points, num_points, -100.0, 100.0, sphere_radius);

    // ����洢
    std::vector<gp_Pnt> projected_tbb(num_points);
    std::vector<gp_Pnt> projected_omp(num_points);
    std::vector<gp_Pnt> projected_serial(num_points);

    // ����ͶӰ
    auto t1 = std::chrono::high_resolution_clock::now();
    project_points_serial_fast(points, projected_serial, geomSphere);
    auto t2 = std::chrono::high_resolution_clock::now();
    double serial_time = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "����ͶӰ��ʱ: " << serial_time << " ��" << std::endl;

    // TBB����ͶӰ
    t1 = std::chrono::high_resolution_clock::now();
    project_points_tbb_fast(points, projected_tbb, geomSphere, num_threads);
    t2 = std::chrono::high_resolution_clock::now();
    double tbb_time = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "TBB����ͶӰ��ʱ: " << tbb_time << " ��" << std::endl;

    // OpenMP����ͶӰ
    t1 = std::chrono::high_resolution_clock::now();
    project_points_omp_fast(points, projected_omp, geomSphere, num_threads);
    t2 = std::chrono::high_resolution_clock::now();
    double omp_time = std::chrono::duration<double>(t2 - t1).count();
    std::cout << "OpenMP����ͶӰ��ʱ: " << omp_time << " ��" << std::endl;

    // ���TBBͶӰ���Ƿ��������ϣ�������Ľ������ʽ��
    int tbb_not_on_sphere = 0;
    for (const auto &pt : projected_tbb)
    {
        double dist = std::sqrt(pt.X() * pt.X() + pt.Y() * pt.Y() + pt.Z() * pt.Z());
        if (std::abs(dist - sphere_radius) > 1e-8)
        {
            ++tbb_not_on_sphere;
        }
    }
    std::cout << "TBBͶӰ�㲻�������ϵ�����: " << tbb_not_on_sphere << std::endl;

    // ���OpenMPͶӰ���Ƿ���������
    int omp_not_on_sphere = 0;
    for (const auto &pt : projected_omp)
    {
        double dist = std::sqrt(pt.X() * pt.X() + pt.Y() * pt.Y() + pt.Z() * pt.Z());
        if (std::abs(dist - sphere_radius) > 1e-8)
        {
            ++omp_not_on_sphere;
        }
    }
    std::cout << "OpenMPͶӰ�㲻�������ϵ�����: " << omp_not_on_sphere << std::endl;

    // ��֤����ͶӰ�봮��ͶӰ���һ����
    int mismatch_tbb = 0, mismatch_omp = 0;
    for (int i = 0; i < num_points; ++i)
    {
        if (projected_serial[i].Distance(projected_tbb[i]) > 1e-8)
            ++mismatch_tbb;
        if (projected_serial[i].Distance(projected_omp[i]) > 1e-8)
            ++mismatch_omp;
    }
    std::cout << "������TBBͶӰ�����һ�µ���: " << mismatch_tbb << std::endl;
    std::cout << "������OpenMPͶӰ�����һ�µ���: " << mismatch_omp << std::endl;

    // ������ٱȺͲ���Ч��
    double tbb_speedup = serial_time / tbb_time;
    double omp_speedup = serial_time / omp_time;
    double tbb_efficiency = tbb_speedup / num_threads * 100.0;
    double omp_efficiency = omp_speedup / num_threads * 100.0;
    
    // ������ܶԱȱ��
    std::cout << "\n=================== ����ͶӰ���ܶԱ� ==============" << std::endl;
    std::cout << "��ʽ      | �߳��� |   ��ʱ(s) |  ���ٱ� | ����Ч��(%)" << std::endl;
    std::cout << "----------------------------------------------------" << std::endl;
    std::cout << "����    | "
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