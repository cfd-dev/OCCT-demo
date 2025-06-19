// OpenCASCADE和标准库头文件
#include <iostream>                        // 输入输出流库
#include <Standard_Version.hxx>            // OpenCASCADE版本信息
#include <vtkVersion.h>                    // VTK版本信息
#include <gp_Circ.hxx>                     // 几何基本圆定义
#include <Geom_Circle.hxx>                 // 几何圆曲线定义
#include <GeomAPI_ProjectPointOnCurve.hxx> // 点投影到曲线API
#include <BRepBuilderAPI_MakeVertex.hxx>   // 顶点创建API
#include <BRepBuilderAPI_MakeEdge.hxx>     // 边创建API
#include <TopoDS_Shape.hxx>                // 拓扑数据结构定义
#include <V3d_Viewer.hxx>                  // 3D查看器
#include <AIS_InteractiveContext.hxx>      // 交互上下文
#include <WNT_Window.hxx>                  // Windows本地窗口
#include <windows.h>                       // Windows API
#include <OpenGl_GraphicDriver.hxx>        // OpenGL图形驱动
#include <TopoDS_Edge.hxx>                 // 拓扑边结构
#include <AIS_Shape.hxx>                   // 交互式形状
#include <V3d_View.hxx>                    // 3D视图

int main()
{
    // 输出库版本信息
    std::cout << "OpenCASCADE Version: "
              << OCC_VERSION_MAJOR << "."
              << OCC_VERSION_MINOR << "."
              << OCC_VERSION_MAINTENANCE
              << std::endl;
    std::cout << "VTK Version: "
              << vtkVersion::GetVTKMajorVersion() << "."
              << vtkVersion::GetVTKMinorVersion() << "."
              << vtkVersion::GetVTKBuildVersion()
              << std::endl;

    // 创建几何圆（XY平面，半径50）
    gp_Ax2 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));            // 定义坐标系（原点，Z轴方向）
    gp_Circ circle(axis, 50.0);                               // 创建参数化圆
    Handle(Geom_Circle) geomCircle = new Geom_Circle(circle); // 转换为几何曲线

    // 创建测试点
    gp_Pnt point1(30, 40, 10); // 原始点1（不在圆平面上）
    gp_Pnt point2(60, 0, 0);   // 原始点2（超出圆半径）

    // 初始化投影器
    GeomAPI_ProjectPointOnCurve projector;

    // 投影第一个点
    projector.Init(point1, geomCircle); // 设置投影目标和曲线
    if (projector.NbPoints() < 1)       // 检查投影结果
    {
        std::cerr << "Error projecting point1" << std::endl;
        return 1;
    }
    gp_Pnt projectedPoint1 = projector.NearestPoint(); // 获取最近投影点

    // 投影第二个点
    projector.Init(point2, geomCircle);
    if (projector.NbPoints() < 1)
    {
        std::cerr << "Error projecting point2" << std::endl;
        return 1;
    }
    gp_Pnt projectedPoint2 = projector.NearestPoint();

    // 初始化可视化系统
    Handle(OpenGl_GraphicDriver) graphicDriver = new OpenGl_GraphicDriver(NULL); // OpenGL驱动
    Handle(V3d_Viewer) viewer = new V3d_Viewer(graphicDriver);                   // 3D查看器
    Handle(AIS_InteractiveContext) context = new AIS_InteractiveContext(viewer); // 交互上下文

    // 创建并显示圆边
    TopoDS_Edge circleEdge = BRepBuilderAPI_MakeEdge(geomCircle);      // 创建拓扑边
    Handle(AIS_Shape) aisCircle = new AIS_Shape(circleEdge);           // 创建交互式形状
    context->SetColor(aisCircle, Quantity_NOC_YELLOW, Standard_False); // 设置黄色
    context->Display(aisCircle, Standard_True);                        // 显示形状

    // 创建顶点拓扑结构
    TopoDS_Vertex vertex1 = BRepBuilderAPI_MakeVertex(point1);            // 原始点1顶点
    TopoDS_Vertex vertex2 = BRepBuilderAPI_MakeVertex(point2);            // 原始点2顶点
    TopoDS_Vertex projVert1 = BRepBuilderAPI_MakeVertex(projectedPoint1); // 投影点1
    TopoDS_Vertex projVert2 = BRepBuilderAPI_MakeVertex(projectedPoint2); // 投影点2

    // 显示原始点（红色）
    Handle(AIS_Shape) aisPoint1 = new AIS_Shape(vertex1);
    Handle(AIS_Shape) aisPoint2 = new AIS_Shape(vertex2);
    context->SetColor(aisPoint1, Quantity_NOC_RED, Standard_False); // 设置红色
    context->SetColor(aisPoint2, Quantity_NOC_RED, Standard_False);
    context->Display(aisPoint1, Standard_True); // 显示点1
    context->Display(aisPoint2, Standard_True); // 显示点2

    // 显示投影点（绿色）
    Handle(AIS_Shape) aisProj1 = new AIS_Shape(projVert1);
    Handle(AIS_Shape) aisProj2 = new AIS_Shape(projVert2);
    context->SetColor(aisProj1, Quantity_NOC_GREEN, Standard_False); // 设置绿色
    context->SetColor(aisProj2, Quantity_NOC_GREEN, Standard_False);
    context->Display(aisProj1, Standard_True); // 显示投影点1
    context->Display(aisProj2, Standard_True); // 显示投影点2

    // 创建并配置3D视图
    HWND hwnd = GetConsoleWindow(); // 获取控制台窗口句柄
    if (hwnd == NULL)
    {
        std::cerr << "Error: Could not get console window" << std::endl;
        return 1;
    }
    Handle(WNT_Window) window = new WNT_Window(hwnd); // 创建Windows窗口
    Handle(V3d_View) view = viewer->CreateView();     // 创建3D视图
    view->SetWindow(window);                          // 关联窗口
    if (!window->IsMapped())                          // 检查窗口映射状态
    {
        window->Map(); // 映射窗口到屏幕
    }
    view->FitAll(); // 自动调整视图
    view->Redraw(); // 重绘视图

    // 保持程序运行
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.ignore(); // 等待用户输入
    return 0;
}
