#include <iostream>
#include <gp_Sphere.hxx>
#include <Geom_SphericalSurface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <WNT_Window.hxx>
#include <windows.h>
#include <OpenGl_GraphicDriver.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>

int main()
{
    // 创建几何球体（半径50）
    gp_Ax3 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
    gp_Sphere sphere(axis, 50.0);
    Handle(Geom_SphericalSurface) geomSphere = new Geom_SphericalSurface(sphere);

    // 创建三维测试点
    gp_Pnt point1(20, 40, 30); // 球内点
    gp_Pnt point2(60, 0, 40);  // 球外点
    std::cout << "Point 1: " << point1.X() << ", " << point1.Y() << ", " << point1.Z() << std::endl;
    std::cout << "Point 2: " << point2.X() << ", " << point2.Y() << ", " << point2.Z() << std::endl;

    // 初始化曲面投影器
    GeomAPI_ProjectPointOnSurf projector;

    // 投影第一个点
    projector.Init(point1, geomSphere);
    if (projector.NbPoints() < 1) {
        std::cerr << "Error projecting point1" << std::endl;
        return 1;
    }
    gp_Pnt projectedPoint1 = projector.NearestPoint();
    Standard_Real dist1 = projector.LowerDistance();
    Standard_Real u1, v1;
    projector.LowerDistanceParameters(u1, v1);
    gp_Pnt spherePoint1 = geomSphere->Value(u1, v1);
    
    std::cout << "Projected Point 1: " << projectedPoint1.X() << ", " 
              << projectedPoint1.Y() << ", " << projectedPoint1.Z() << std::endl;
    std::cout << "Distance: " << dist1 << std::endl;
    std::cout << "UV Parameters: (" << u1 << ", " << v1 << ")" << std::endl;
    std::cout << "Sphere Point 1: " << spherePoint1.X() << ", " 
              << spherePoint1.Y() << ", " << spherePoint1.Z() << std::endl;
    
    // 投影第二个点
    projector.Init(point2, geomSphere);
    if (projector.NbPoints() < 1) {
        std::cerr << "Error projecting point2" << std::endl;
        return 1;
    }
    gp_Pnt projectedPoint2 = projector.NearestPoint();
    Standard_Real dist2 = projector.LowerDistance();
    Standard_Real u2, v2;
    projector.LowerDistanceParameters(u2, v2);
    gp_Pnt spherePoint2 = geomSphere->Value(u2, v2);
    
    std::cout << "Projected Point 2: " << projectedPoint2.X() << ", " 
              << projectedPoint2.Y() << ", " << projectedPoint2.Z() << std::endl;
    
    std::cout << "Distance: " << dist2 << std::endl;
    std::cout << "UV Parameters: (" << u2 << ", " << v2 << ")" << std::endl;
    std::cout << "Sphere Point 2: " << spherePoint2.X() << ", " 
              << spherePoint2.Y() << ", " << spherePoint2.Z() << std::endl;

    // 初始化可视化系统
    Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(NULL);
    Handle(V3d_Viewer) viewer = new V3d_Viewer(graphicDriver);
    Handle(AIS_InteractiveContext) context = new AIS_InteractiveContext(viewer);

    // 创建并显示球体
    // 从gp_Sphere中提取构造参数
    gp_Pnt center = sphere.Location();
    Standard_Real radius = sphere.Radius();
    BRepPrimAPI_MakeSphere mkSphere(center, radius);
    TopoDS_Shape sphereShape = mkSphere.Shape();
    Handle(AIS_Shape) aisSphere = new AIS_Shape(sphereShape);
    context->SetColor(aisSphere, Quantity_NOC_YELLOW, Standard_False);
    context->Display(aisSphere, Standard_True);
    context->SetDisplayMode(aisSphere, 1, Standard_True);

    // 创建并显示原始点（红色）
    Handle(AIS_Shape) aisPoint1 = new AIS_Shape(BRepBuilderAPI_MakeVertex(point1));
    Handle(AIS_Shape) aisPoint2 = new AIS_Shape(BRepBuilderAPI_MakeVertex(point2));
    context->SetColor(aisPoint1, Quantity_NOC_RED, Standard_False);
    context->SetColor(aisPoint2, Quantity_NOC_RED, Standard_False);
    context->Display(aisPoint1, Standard_True);
    context->Display(aisPoint2, Standard_True);

    // 显示投影点（绿色）
    Handle(AIS_Shape) aisProj1 = new AIS_Shape(BRepBuilderAPI_MakeVertex(projectedPoint1));
    Handle(AIS_Shape) aisProj2 = new AIS_Shape(BRepBuilderAPI_MakeVertex(projectedPoint2));
    context->SetColor(aisProj1, Quantity_NOC_GREEN, Standard_False);
    context->SetColor(aisProj2, Quantity_NOC_GREEN, Standard_False);
    context->Display(aisProj1, Standard_True);
    context->Display(aisProj2, Standard_True);

    // 创建3D视图
    HWND hwnd = GetConsoleWindow();
    Handle(WNT_Window) window = new WNT_Window(hwnd);
    Handle(V3d_View) view = viewer->CreateView();
    view->SetWindow(window);
    view->SetProj(V3d_XposYposZpos);
    view->SetTwist(0.0);
    if (!window->IsMapped()) window->Map();
    view->FitAll();
    view->Redraw();

    // 保持程序运行
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.ignore();
    return 0;
}
