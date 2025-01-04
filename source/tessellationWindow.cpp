#include "tessellationWindow.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gli/gli.hpp>
#undef GLM_ENABLE_EXPERIMENTAL

#include <iostream>

std::string tessellationWindow::m_shaderBasePath = "shaders/";

tessellationWindow::tessellationWindow()
{}

tessellationWindow::~tessellationWindow()
{}

void tessellationWindow::RunInit()
{
    // GLFW
    // *=*=*=*=*=*=*=*=*=*=
    SetUpWindowsCallbacks();
    m_camera.m_farPlane = 100.0f;
    // Scene
    // *=*=*=*=*=*=*=*=*=*=
    m_windowState = wState::DEFALUT;
    m_camera.UpdateRotation(0.0f, glm::radians(-45.0f));

    m_obj_quad.InitGL();
    
    m_sh_quad.Init();
    m_sh_quad.AttachShader(shPath("quad.vert"), GL_VERTEX_SHADER);
    m_sh_quad.AttachShader(shPath("quad.tesc"), GL_TESS_CONTROL_SHADER);
    m_sh_quad.AttachShader(shPath("quad.tese"), GL_TESS_EVALUATION_SHADER);
    m_sh_quad.AttachShader(shPath("quad.frag"), GL_FRAGMENT_SHADER);
    m_sh_quad.Link();

    /* Set Light parameters */
    m_sh_quad.Use();
    m_sh_quad.set3fv("mat.ka", glm::vec3(0.1f, 0.1f, 0.1f));
    m_sh_quad.set3fv("mat.kd", glm::vec3(0.8f, 0.8f, 0.8f));
    m_sh_quad.set3fv("mat.ks", glm::vec3(1.0f, 1.0f, 1.0f));
    m_sh_quad.set1f("mat.shininess", 32.0f);
    m_sh_quad.set3fv("lgt.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
    m_sh_quad.set3fv("lgt.diffuse", glm::vec3(0.0855f, 0.287f, 0.950f));
    m_sh_quad.set3fv("lgt.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    m_sh_quad.set3fv("lgt.position", glm::vec3(0.0f, 5.0f, 0.0f));

    m_sh_quad.set1i("NumOfPatches", NUM_SIDE_PATCHES);
    m_sh_quad.set1i("TEX_diffuse", 0);
    m_sh_quad.set1i("TEX_normals", 1);
    m_sh_quad.set1i("TEX_height", 2);

    m_obj_controlPoints.InitGL();

    m_sh_controlPoints.Init();
    m_sh_controlPoints.AttachShader(shPath("controlPoints.vert"), GL_VERTEX_SHADER);
    m_sh_controlPoints.AttachShader(shPath("controlPoints.frag"), GL_FRAGMENT_SHADER);
    m_sh_controlPoints.Link();

    InitKeyboardMenager();
    PreparePatchesModelMatrices();

    // textures
    m_tex_diffuse = LoadDDSTextureFromFile("resources/textures/tess/diffuse.dds");
    m_tex_height = LoadDDSTextureFromFile("resources/textures/tess/height.dds");
    m_tex_normals = LoadDDSTextureFromFile("resources/textures/tess/normals.dds");

    // GUI
    // *=*=*=*=*=*=*=*=*=*=
    const GLubyte* renderer = glGetString(GL_RENDERER);
    m_gui_renderer = std::string((const char*)renderer);
    
    m_gui_hintColor = ImVec4(0.231f, 0.820f, 0.0984f, 1.0f);

    // OpenGL
    // *=*=*=*=*=*=*=*=*=*=

    // UBOs
    m_MatriciesUBO.CreateUBO(2 * sizeof(glm::mat4));
    m_MatriciesUBO.BindBufferBaseToBindingPoint(0);
    m_ControlPointsUBO.CreateUBO(16 * CONTROL_POINTS_SETS * sizeof(glm::vec4));
    m_ControlPointsUBO.BindBufferBaseToBindingPoint(1);

    m_sh_quad.BindUniformBlockToBindingPoint("Matrices", 0);
    m_sh_quad.BindUniformBlockToBindingPoint("ControlPoints", 1);
    m_sh_controlPoints.BindUniformBlockToBindingPoint("ControlPoints", 1);

    PopulateBezierPointsUBO();

    // Tessellation params
    m_tessLevelOuter = glm::vec4(10.0f, 10.0f, 10.0f, 10.0f);
    m_tessLevelInner = glm::vec2(10.0f, 10.0f);
    m_tessUniformValue = 10.0f;

    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void tessellationWindow::RunRenderTick()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update UBO
    float aspect = static_cast<float>(m_width)/m_height;
    glm::mat4 matrices[2] = {m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix(aspect)};
    m_MatriciesUBO.BindUBO();
    m_MatriciesUBO.SetBufferData(0, matrices, 2 * sizeof(glm::mat4));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex_diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_tex_normals);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_tex_height);

    /* 
        DRAW PATCHES 
    */
    m_sh_quad.Use();
    m_sh_quad.set4fv("outerLevel", m_tessLevelOuter);
    m_sh_quad.set2fv("innerLevel", m_tessLevelInner);
    m_sh_quad.set3fv("viewPos", m_camera.m_worldPos);
    m_sh_quad.set1b("UsePhong", m_bUsePhong);
    m_sh_quad.set1b("DynamicLoD", m_bDynamicLoD);
    m_sh_quad.set1b("Texturing", m_bTexturing);
    m_sh_quad.set1b("NormalMapping", m_bNormalMappping);
    m_sh_quad.set1b("DisplacementMapping", m_bDisplacementMapping);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    if (m_bDisplayPatches) {
        for (int i = 0; i < NUM_SIDE_PATCHES * NUM_SIDE_PATCHES; ++i) {
            m_sh_quad.setM4fv("model", GL_FALSE, m_modelMatrices[i]);
            m_sh_quad.set1i("PatchIndex", i);

            if (m_bezierShape == 0) {
                m_sh_quad.set1i("bezierShape", m_bezierShape);
            } else if (m_bezierShape == 1) {
                m_sh_quad.set1i("bezierShape", m_convexPatches[i]);
            } else if (m_bezierShape == 3) {
                m_sh_quad.set1i("bezierShape", m_wavesPatches[i]);
            }

            m_obj_quad.Draw();
        }
    }
    else  {
        m_sh_quad.set1i("bezierShape", m_bezierShape);
        m_sh_quad.setM4fv("model", GL_FALSE, glm::mat4(1.0f));
        m_obj_quad.Draw();
    }

    /* 
        DRAW CONTROL POINTS OF THE ZERO PATCH 
    */

    if (m_bShowControlPoints) {
        m_sh_controlPoints.Use();
        m_sh_controlPoints.set1i("bezierShape", m_bezierShape);
        if (m_bDisplayPatches) {
            m_sh_controlPoints.setM4fv("model", GL_FALSE, m_modelMatrices[0]);
        }
        else {
            m_sh_controlPoints.setM4fv("model", GL_FALSE, glm::mat4(1.0f));
        }
        m_obj_controlPoints.Draw();
    }

    RenderGUI();
}

void tessellationWindow::RunClear()
{
    m_obj_quad.DeInitGL();
    m_sh_quad.DeInitGL();

    m_obj_controlPoints.DeInitGL();
    m_sh_controlPoints.DeInitGL();
}

std::string tessellationWindow::shPath(std::string fileName)
{
    return m_shaderBasePath + fileName;
}

void tessellationWindow::PopulateBezierPointsUBO()
{
    std::vector<glm::vec4> points;
    points.reserve(CONTROL_POINTS_SETS * 16);

    float d = 2.0f / 3;

    // flat
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            points.push_back({
                -1.0f + d * i, 
                0.0f, 
                1.0f - d * j, 
                1.0f,
            });
        }
    }

    // down 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            points.push_back({
                -1.0f + d * i, 
                (i == 0 || j == 0 || i == 3 || j == 3) ? 0.0f : -1.0f, 
                1.0f - d * j, 
                1.0f
            });
        }
    }
    // up 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            points.push_back({
                -1.0f + d * i, 
                (i == 0 || j == 0 || i == 3 || j == 3) ? 0.0f : 1.0f, 
                1.0f - d * j, 
                1.0f
            });
        }
    }
    // wave up 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            points.push_back({
                -1.0f + d * i, 
                (i == 0 || i == 3) ? 0.0f : 0.5f, 
                1.0f - d * j, 
                1.0f
            });
        }
    }
    //wave down
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            points.push_back({
                -1.0f + d * i, 
                (i == 0 || i == 3) ? 0.0f : -0.5f, 
                1.0f - d * j, 
                1.0f
            });
        }
    }

    m_ControlPointsUBO.BindUBO();
    m_ControlPointsUBO.SetBufferData(0, points.data(), sizeof(glm::vec4) * points.size());
}

void tessellationWindow::RenderGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, -1), ImVec2(FLT_MAX, -1), &tessellationWindow::InfoWindowSizeCallback, (void*)this);
    ImGui::Begin("Project: Tessellation", (bool*)0, ImGuiWindowFlags_NoCollapse);
    GenGUI_AppStatistics();
    GenGUI_Tessellation();
    // ImGui::ShowDemoWindow();
    ImGui::End();

    // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

}

void tessellationWindow::InfoWindowSizeCallback(ImGuiSizeCallbackData* data)
{
    tessellationWindow* win = reinterpret_cast<tessellationWindow*>(data->UserData);
    data->DesiredSize.y = win->m_height;
    win->m_gui_menuWidth = data->CurrentSize.x;
}


void tessellationWindow::GenGUI_AppStatistics()
{
    m_gui_fps.Update(m_deltaTime);

    static bool fpsLimit = true;
    bool prevFpsLimit = fpsLimit;

    const char* swapIntervals[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    static int currentSwapInterval = 0;

    if(ImGui::CollapsingHeader("App Statistics"))
    {
        ImGui::TextColored(m_gui_hintColor, m_gui_renderer.c_str());

        if (ImGui::BeginTable("split", 2))
        {
            ImGui::TableNextColumn(); 
            ImGui::Text("FPS = %.2f", m_gui_fps.GetAvarageFPS());
            ImGui::TableNextColumn(); 
            ImGui::Checkbox("FPS limit", &fpsLimit);
            if (ImGui::BeginItemTooltip())
            {
                ImGui::TextColored(m_gui_hintColor, "With the limit maximum FSP number is bounded by a screen refresh rate.");
                ImGui::TextColored(m_gui_hintColor, "Without the limit maximum GPU and CPU generate as much FPS as it possible");
                ImGui::EndTooltip();
            }
            ImGui::TableNextColumn(); 
            ImGui::Text("MS = %.6f", m_gui_fps.GetRenderTime());

            if (prevFpsLimit != fpsLimit) {
                glfwSwapInterval(fpsLimit ? currentSwapInterval + 1 : 0);
                m_gui_fps.Reset();
            }

            ImGui::TableNextColumn(); 
            ImGui::BeginDisabled(!fpsLimit);
            if (ImGui::BeginCombo("swap", swapIntervals[currentSwapInterval])) 
            {
                for (int i = 0; i < IM_ARRAYSIZE(swapIntervals); i++) {
                    const bool is_selected = (i == currentSwapInterval);
                    if (ImGui::Selectable(swapIntervals[i], is_selected)) 
                    {
                        currentSwapInterval = i;
                        glfwSwapInterval(currentSwapInterval + 1);
                        m_gui_fps.Reset();
                    }

                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            } 
            ImGui::EndDisabled();
            
            ImGui::EndTable();
        }
    }
}

void tessellationWindow::GenGUI_Tessellation()
{
    if(ImGui::CollapsingHeader("Tessellation"))
    {   
        ImGui::SeparatorText("Parameters");

        ImGui::BeginDisabled(m_bDynamicLoD);
        glm::vec4 prevTessOuter = m_tessLevelOuter;
        if (ImGui::DragFloat4("outer", reinterpret_cast<float*>(&m_tessLevelOuter), 0.1f, 1.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
        {   
            // When display patches is turned on, outer tessellation 
            // [0] = [2] and [1] = [3], so inner patches don't have 
            // any holes.
            if (m_bDisplayPatches)
            {
                if (prevTessOuter.x != m_tessLevelOuter.x) {
                    m_tessLevelOuter.z = m_tessLevelOuter.x;
                } else if (prevTessOuter.z != m_tessLevelOuter.z) {
                    m_tessLevelOuter.x = m_tessLevelOuter.z;
                } else if (prevTessOuter.y != m_tessLevelOuter.y) {
                    m_tessLevelOuter.w = m_tessLevelOuter.y;
                } else if (prevTessOuter.w != m_tessLevelOuter.w) {
                    m_tessLevelOuter.y = m_tessLevelOuter.w;
                }
            }
        }
        ImGui::DragFloat2("inner", reinterpret_cast<float*>(&m_tessLevelInner), 0.1f, 1.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp); 
        if (ImGui::DragFloat("uniform value", &m_tessUniformValue, 0.1f, 1.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp)) 
        {
            m_tessLevelOuter = glm::vec4(m_tessUniformValue);
            m_tessLevelInner = glm::vec2(m_tessUniformValue);
        }
        ImGui::EndDisabled();

        ImGui::SeparatorText("Keyboard");
        if (ImGui::BeginTable("Tessellation settings", 3, 
                    ImGuiTableFlags_BordersH |
                    ImGuiTableFlags_BordersV)) 
        {
            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 23.0f);
            ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 37.0f);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed, m_gui_menuWidth - 100.0f);
            ImGui::TableHeadersRow();
            
            for (const auto& key : m_keyboardMenager.GetRegisteredKeys()) {
                auto info = m_keyboardMenager.GetCurrentState(key);
                ImGui::TableNextColumn();
                ImGui::SameLine(15); ImGui::TextColored(m_gui_hintColor ,"%c", key);
                ImGui::TableNextColumn();
                ImGui::Text(info.name.c_str());
                ImGui::TableNextColumn();
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + m_gui_menuWidth - 100.0f);
                ImGui::Text(m_keyboardMenager.GetKeyHint(key).c_str());
                ImGui::PopTextWrapPos();
            }   

            ImGui::EndTable();
        }
    }
}

void tessellationWindow::SetUpWindowsCallbacks()
{
    glfwSetMouseButtonCallback(m_window, &tessellationWindow::MouseButtonCallback);
    glfwSetKeyCallback(m_window, &tessellationWindow::KeyCallback);
    glfwSetCursorPosCallback(m_window, &tessellationWindow::CursorPosCallback);
    glfwSetWindowSizeCallback(m_window, &tessellationWindow::WindowSizeCallback);
}

void tessellationWindow::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
    tessellationWindow* win = GW(window);

    win->m_width = width;
    win->m_height = height;
    glViewport(0, 0, width, height);
}

void tessellationWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // (1) GUI callback handling
    // =========================
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (io.WantCaptureKeyboard) {
        return;
    }

    // (2) App callback handling
    // =========================
    tessellationWindow* win = GW(window);

    if (action == GLFW_PRESS) {
        win->m_keyboardMenager.GoToNextState(key);
    }
}

void tessellationWindow::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    // (1) GUI callback handling
    // =========================
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    if (io.WantCaptureMouse)
        return;

    
    // (2) App callback handling
    // =========================
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_RIGHT:
        MouseButton_RIGHT_Callback(window, action, mods);    
        break;
    case GLFW_MOUSE_BUTTON_LEFT:
        MouseButton_LEFT_Callback(window, action, mods);
        break;

    default:
        break;
    }
}

void tessellationWindow::MouseButton_RIGHT_Callback(GLFWwindow* window, int action, int mods)
{
    tessellationWindow* win = GW(window);

    if (win->GetState() == wState::DEFALUT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        win->m_mousePrevPos.x = xpos;
        win->m_mousePrevPos.y = ypos;

        win->SetState(wState::CAMERA_MOVE);
    }
    else if (win->GetState() == wState::CAMERA_MOVE && action == GLFW_RELEASE) {
        win->SetState(wState::DEFALUT);
    }
}

void tessellationWindow::MouseButton_LEFT_Callback(GLFWwindow* window, int action, int mods)
{
    tessellationWindow* win = GW(window);

    if (win->GetState() == wState::DEFALUT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        win->m_mousePrevPos.x = xpos;
        win->m_mousePrevPos.y = ypos;

        win->SetState(wState::CAMERA_ROTATE);
    }
    else if (win->GetState() == wState::CAMERA_ROTATE && action == GLFW_RELEASE) {
        win->SetState(wState::DEFALUT);
    }
}

void tessellationWindow::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // (1) GUI callback handling
    // =========================
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(xpos, ypos);
    if (io.WantCaptureMouse)
        return;

    // (2) App callback handling
    // =========================
    tessellationWindow* win = GW(window);

    float deltaX = xpos - win->m_mousePrevPos.x;
    float deltaY = ypos - win->m_mousePrevPos.y;
    win->m_mousePrevPos.x = xpos;
    win->m_mousePrevPos.y = ypos;

    switch (win->GetState())
    {
    case wState::CAMERA_MOVE:
        {
            win->m_camera.UpdatePosition(deltaY * win->m_camera.m_cameraSpeed);
            glm::vec3 v = win->m_camera.m_worldPos;
        }
        break;
    
    case wState::CAMERA_ROTATE:
        win->m_camera.UpdateRotation(-deltaX * win->m_camera.m_cameraSpeed, -deltaY * win->m_camera.m_cameraSpeed);
        break;

    default:
        break;
    }
}

tessellationWindow* tessellationWindow::GW(GLFWwindow* window)
{
    return reinterpret_cast<tessellationWindow*>(glfwGetWindowUserPointer(window));
}

void tessellationWindow::InitKeyboardMenager()
{
    m_keyboardMenager.RegisterKey(GLFW_KEY_W, "grid view")
    .AddState("On", std::bind(&tessellationWindow::SetPolyMode, this, std::placeholders::_1))
    .AddState("Off", std::bind(&tessellationWindow::SetPolyMode, this, std::placeholders::_1));
    SetPolyMode(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_F, "Bezier control points' shape")
    .AddState("Flat", std::bind(&tessellationWindow::SetBezierPointsShape, this, std::placeholders::_1))
    .AddState("Convex", std::bind(&tessellationWindow::SetBezierPointsShape, this, std::placeholders::_1))
    .AddState("Wave", std::bind(&tessellationWindow::SetBezierPointsShape, this, std::placeholders::_1));
    SetBezierPointsShape(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_C, "Show Bezier control points")
    .AddState("Off", std::bind(&tessellationWindow::SetShowBezierPoints, this, std::placeholders::_1))
    .AddState("On", std::bind(&tessellationWindow::SetShowBezierPoints, this, std::placeholders::_1));
    SetShowBezierPoints(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_S, "Phong Shading of the surface")
    .AddState("Off", std::bind(&tessellationWindow::SetPhongShading, this, std::placeholders::_1))
    .AddState("On",  std::bind(&tessellationWindow::SetPhongShading, this, std::placeholders::_1));
    SetPhongShading(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_Q, "Display 16 Bezier Patches")
    .AddState("Off", std::bind(&tessellationWindow::SetDisplayPatches, this, std::placeholders::_1))
    .AddState("On", std::bind(&tessellationWindow::SetDisplayPatches, this, std::placeholders::_1));
    SetDisplayPatches(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_E, "Dynamic level of details")
    .AddState("Off", std::bind(&tessellationWindow::SetDynamicLoD, this, std::placeholders::_1))
    .AddState("On", std::bind(&tessellationWindow::SetDynamicLoD, this, std::placeholders::_1));
    SetDynamicLoD(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_T, "Surface texturing")
    .AddState("Off", std::bind(&tessellationWindow::SetTexturing, this, std::placeholders::_1))
    .AddState("On", std::bind(&tessellationWindow::SetTexturing, this, std::placeholders::_1));
    SetTexturing(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_R, "Normal mapping")
    .AddState("Off", std::bind(&tessellationWindow::SetNormalMapping, this, std::placeholders::_1))
    .AddState("On", std::bind(&tessellationWindow::SetNormalMapping, this, std::placeholders::_1));
    SetNormalMapping(0);

    m_keyboardMenager.RegisterKey(GLFW_KEY_D, "Displacement mapping")
    .AddState("Off", std::bind(&tessellationWindow::SetDisplacementMapping, this, std::placeholders::_1))
    .AddState("On", std::bind(&tessellationWindow::SetDisplacementMapping, this, std::placeholders::_1));
    SetDisplacementMapping(0);
}

void tessellationWindow::SetPolyMode(unsigned i)
{
    glPolygonMode(GL_FRONT_AND_BACK, i == 0 ? GL_LINE : GL_FILL);
}

void tessellationWindow::SetBezierPointsShape(unsigned i)
{
    switch (i)
    {
    case 0:
        m_bezierShape = 0;
        break;
    case 1:
        m_bezierShape = 1;
        break;
    case 2:
        m_bezierShape = 3;
        break;    
    default:
        break;
    }
}

void tessellationWindow::SetShowBezierPoints(unsigned i)
{
    m_bShowControlPoints = static_cast<bool>(i);
}

void tessellationWindow::SetPhongShading(unsigned i)
{
    m_bUsePhong = static_cast<bool>(i);
}

void tessellationWindow::SetDisplayPatches(unsigned i)
{
    m_bDisplayPatches = static_cast<bool>(i);

    if (m_bDisplayPatches) {
        if (m_tessLevelOuter.x < m_tessLevelOuter.z) {
            m_tessLevelOuter.x = m_tessLevelOuter.z;
        } else {
            m_tessLevelOuter.z = m_tessLevelOuter.x;
        }

        if (m_tessLevelOuter.y < m_tessLevelOuter.w) {
            m_tessLevelOuter.y = m_tessLevelOuter.w;
        } else {
            m_tessLevelOuter.w = m_tessLevelOuter.y;
        }
    }
}

void tessellationWindow::SetDynamicLoD(unsigned i)
{
    m_bDynamicLoD = static_cast<bool>(i); 
}

void tessellationWindow::SetTexturing(unsigned i)
{
    m_bTexturing = static_cast<bool>(i);
}

void tessellationWindow::SetNormalMapping(unsigned i)
{
    m_bNormalMappping = static_cast<bool>(i);
}

void tessellationWindow::SetDisplacementMapping(unsigned i)
{
    m_bDisplacementMapping = static_cast<bool>(i);
}

// From GLI manual
GLuint tessellationWindow::LoadDDSTextureFromFile(const std::string& path)
{
	gli::texture Texture = gli::load(path);
	if(Texture.empty())
    {
        std::cerr << "An error occured during loading DDS texutre" 
        << "\n\t[path]: " << path 
        << std::endl;  

        return 0;
    }

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());

	GLuint TextureName = 0;
	glGenTextures(1, &TextureName);
	glBindTexture(Target, TextureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[0]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[1]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[2]);
	glTexParameteri(Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[3]);

	glm::tvec3<GLsizei> const Extent(Texture.extent());
	GLsizei const FaceTotal = static_cast<GLsizei>(Texture.layers() * Texture.faces());

	switch(Texture.target())
	{
	case gli::TARGET_1D:
		glTexStorage1D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x);
		break;
	case gli::TARGET_1D_ARRAY:
	case gli::TARGET_2D:
	case gli::TARGET_CUBE:
		glTexStorage2D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Texture.target() == gli::TARGET_2D ? Extent.y : FaceTotal);
		break;
	case gli::TARGET_2D_ARRAY:
	case gli::TARGET_3D:
	case gli::TARGET_CUBE_ARRAY:
		glTexStorage3D(
			Target, static_cast<GLint>(Texture.levels()), Format.Internal,
			Extent.x, Extent.y,
			Texture.target() == gli::TARGET_3D ? Extent.z : FaceTotal);
		break;
	default:
		assert(0);
		break;
	}

	for(std::size_t Layer = 0; Layer < Texture.layers(); ++Layer)
	for(std::size_t Face = 0; Face < Texture.faces(); ++Face)
	for(std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		GLsizei const LayerGL = static_cast<GLsizei>(Layer);
		glm::tvec3<GLsizei> Extent(Texture.extent(Level));
		Target = gli::is_target_cube(Texture.target())
			? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face)
			: Target;

		switch(Texture.target())
		{
		case gli::TARGET_1D:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage1D(
					Target, static_cast<GLint>(Level), 0, Extent.x,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_1D_ARRAY:
		case gli::TARGET_2D:
		case gli::TARGET_CUBE:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage2D(
					Target, static_cast<GLint>(Level),
					0, 0,
					Extent.x,
					Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		case gli::TARGET_2D_ARRAY:
		case gli::TARGET_3D:
		case gli::TARGET_CUBE_ARRAY:
			if(gli::is_compressed(Texture.format()))
				glCompressedTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.Internal, static_cast<GLsizei>(Texture.size(Level)),
					Texture.data(Layer, Face, Level));
			else
				glTexSubImage3D(
					Target, static_cast<GLint>(Level),
					0, 0, 0,
					Extent.x, Extent.y,
					Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL,
					Format.External, Format.Type,
					Texture.data(Layer, Face, Level));
			break;
		default: assert(0); break;
		}
	}
	return TextureName;
}

void tessellationWindow::PreparePatchesModelMatrices()
{
    float sideSize = 1.0f;
    float defaultSize = 2.0f;   // Size as defined at VBO;

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(sideSize / defaultSize));
    glm::mat4 translation;

    glm::vec3 start(
        - sideSize * (NUM_SIDE_PATCHES / 2) + (NUM_SIDE_PATCHES % 2 ? 0.0f : sideSize / 2.0f), 
        0.0f,
        sideSize * (NUM_SIDE_PATCHES / 2) - (NUM_SIDE_PATCHES % 2 ? 0.0f : sideSize / 2.0f)
    );

    for (int i = 0; i < NUM_SIDE_PATCHES; ++i) {
        for (int j = 0; j < NUM_SIDE_PATCHES; ++j) {
            translation = glm::translate(glm::mat4(1.0f), 
                glm::vec3(start.x + sideSize * i, 0.0f, start.z - sideSize * j));
            
            m_modelMatrices[NUM_SIDE_PATCHES * i + j] = translation * scale;
        }
    }

    for (int i = 0; i < NUM_SIDE_PATCHES; i++) {
        for (int j = 0; j < NUM_SIDE_PATCHES; j++) {
            m_wavesPatches[NUM_SIDE_PATCHES * i + j] = i % 2 ? 4 : 3;
            m_convexPatches[NUM_SIDE_PATCHES * i + j] = (i + j) % 2 ? 2 : 1;
        }
    }
}

void tessellationWindow::SetState(wState newState)
{
    m_windowState = newState;   
}

tessellationWindow::wState tessellationWindow::GetState()
{
    return m_windowState;
}
