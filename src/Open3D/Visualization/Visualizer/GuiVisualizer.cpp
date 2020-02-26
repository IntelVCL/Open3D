// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include "GuiVisualizer.h"

#include "Open3D/GUI/Application.h"
#include "Open3D/GUI/Button.h"
#include "Open3D/GUI/Checkbox.h"
#include "Open3D/GUI/Color.h"
#include "Open3D/GUI/ColorEdit.h"
#include "Open3D/GUI/Dialog.h"
#include "Open3D/GUI/FileDialog.h"
#include "Open3D/GUI/Label.h"
#include "Open3D/GUI/Layout.h"
#include "Open3D/GUI/SceneWidget.h"
#include "Open3D/GUI/Slider.h"
#include "Open3D/GUI/Theme.h"
#include "Open3D/Geometry/BoundingVolume.h"
#include "Open3D/Geometry/PointCloud.h"
#include "Open3D/Geometry/TriangleMesh.h"
#include "Open3D/IO/ClassIO/PointCloudIO.h"
#include "Open3D/IO/ClassIO/TriangleMeshIO.h"
#include "Open3D/Open3DConfig.h"
#include "Open3D/Utility/Console.h"
#include "Open3D/Utility/FileSystem.h"
#include "Open3D/Visualization/Rendering/RendererStructs.h"
#include "Open3D/Visualization/Rendering/Scene.h"

namespace open3d {
namespace visualization {

namespace {

std::shared_ptr<gui::Dialog> createAboutDialog(gui::Window *window) {
    auto &theme = window->GetTheme();
    auto dlg = std::make_shared<gui::Dialog>("About");

    auto title = std::make_shared<gui::Label>(
            (std::string("Open3D ") + OPEN3D_VERSION).c_str());
    auto text = std::make_shared<gui::Label>(
            "The MIT License (MIT)\n"
            "Copyright (c) 2018 www.open3d.org\n\n"

            "Permission is hereby granted, free of charge, to any person "
            "obtaining "
            "a copy of this software and associated documentation files (the "
            "\"Software\"), to deal in the Software without restriction, "
            "including "
            "without limitation the rights to use, copy, modify, merge, "
            "publish, "
            "distribute, sublicense, and/or sell copies of the Software, and "
            "to "
            "permit persons to whom the Software is furnished to do so, "
            "subject to "
            "the following conditions:\n\n"

            "The above copyright notice and this permission notice shall be "
            "included in all copies or substantial portions of the "
            "Software.\n\n"

            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, "
            "EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES "
            "OF "
            "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND "
            "NONINFRINGEMENT. "
            "IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR "
            "ANY "
            "CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF "
            "CONTRACT, "
            "TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE "
            "SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");
    auto ok = std::make_shared<gui::Button>("OK");
    ok->SetOnClicked([window]() { window->CloseDialog(); });

    gui::Margins margins(theme.fontSize);
    auto layout = std::make_shared<gui::Vert>(0, margins);
    layout->AddChild(gui::Horiz::MakeCentered(title));
    layout->AddChild(gui::Horiz::MakeFixed(theme.fontSize));
    layout->AddChild(text);
    layout->AddChild(gui::Horiz::MakeFixed(theme.fontSize));
    layout->AddChild(gui::Horiz::MakeCentered(ok));
    dlg->AddChild(layout);

    return dlg;
}

std::shared_ptr<gui::Dialog> createContactDialog(gui::Window *window) {
    auto &theme = window->GetTheme();
    auto em = theme.fontSize;
    auto dlg = std::make_shared<gui::Dialog>("Contact Us");

    auto title = std::make_shared<gui::Label>("Contact Us");
    auto leftCol = std::make_shared<gui::Label>(
            "Web site:\n"
            "Code:\n"
            "Mailing list:\n"
            "Discord channel:");
    auto rightCol = std::make_shared<gui::Label>(
            "http://www.open3d.org\n"
            "http://github.org/intel-isl/Open3D\n"
            "http://www.open3d.org/index.php/subscribe/\n"
            "https://discord.gg/D35BGvn");
    auto ok = std::make_shared<gui::Button>("OK");
    ok->SetOnClicked([window]() { window->CloseDialog(); });

    gui::Margins margins(em);
    auto layout = std::make_shared<gui::Vert>(0, margins);
    layout->AddChild(gui::Horiz::MakeCentered(title));
    layout->AddChild(gui::Horiz::MakeFixed(em));

    auto columns = std::make_shared<gui::Horiz>(em, gui::Margins());
    columns->AddChild(leftCol);
    columns->AddChild(rightCol);
    layout->AddChild(columns);

    layout->AddChild(gui::Horiz::MakeFixed(em));
    layout->AddChild(gui::Horiz::MakeCentered(ok));
    dlg->AddChild(layout);

    return dlg;
}

std::shared_ptr<gui::Slider> AddAttributeSlider(const gui::Slider::Type type,
                                                const double min,
                                                const double max,
                                                const double value,
                                                const char *caption,
                                                gui::Widget *parent) {
    auto slider = std::make_shared<gui::Slider>(type);
    slider->SetLimits(min, max);
    slider->SetValue(value);
    parent->AddChild(slider);

    auto lblCaption = std::make_shared<gui::Label>(caption);
    parent->AddChild(lblCaption);

    return slider;
}

}  // namespace

enum MenuId {
    FILE_OPEN,
    FILE_EXPORT_RGB,
    FILE_EXPORT_DEPTH,
    FILE_CLOSE,
    VIEW_POINTS,
    VIEW_WIREFRAME,
    VIEW_MESH,
    SETTINGS_DEFAULT_MATERIAL,
    SETTINGS_LIGHTNING,
    HELP_ABOUT,
    HELP_CONTACT
};

struct GuiVisualizer::Impl {
    std::vector<visualization::GeometryHandle> geometryHandles;

    std::shared_ptr<gui::SceneWidget> scene;
    std::shared_ptr<gui::Horiz> bottomBar;

    struct LightSettings {
        visualization::IndirectLightHandle hIbl;
        visualization::SkyboxHandle hSky;
        visualization::LightHandle hDirectionalLight;

        std::shared_ptr<gui::Widget> wgtBase;
        std::shared_ptr<gui::Button> wgtLoadAmbient;
        std::shared_ptr<gui::Button> wgtLoadSky;
        std::shared_ptr<gui::Checkbox> wgtAmbientEnabled;
        std::shared_ptr<gui::Checkbox> wgtSkyEnabled;
        std::shared_ptr<gui::Checkbox> wgtDirectionalEnabled;
        std::shared_ptr<gui::Slider> wgtIntensity;
        std::shared_ptr<gui::Slider> wgtAmbientIntensity;
        std::shared_ptr<gui::ColorEdit> wgtLightColor;
    } lightSettings;

    // TBD
    //    struct LitMaterialSettings {
    //        visualization::MaterialInstanceHandle hMaterialInstance;
    //
    //        std::shared_ptr<gui::Widget> wgtBase;
    //        std::shared_ptr<gui::ColorEdit> wgtBaseColor;
    //        std::shared_ptr<gui::Slider> wgtRoughness;
    //        std::shared_ptr<gui::Checkbox> wgtMetallic;
    //        std::shared_ptr<gui::Slider> wgtReflectance;
    //        std::shared_ptr<gui::Slider> wgtClearCoat;
    //        std::shared_ptr<gui::Slider> wgtClearCoatRoughness;
    //        std::shared_ptr<gui::Slider> wgtAnisotropy;
    //    } litMaterialSettings;
};

GuiVisualizer::GuiVisualizer(
        const std::vector<std::shared_ptr<const geometry::Geometry>>
                &geometries,
        const std::string &title,
        int width,
        int height,
        int left,
        int top)
    : gui::Window(title, left, top, width, height),
      impl_(new GuiVisualizer::Impl()) {
    auto &app = gui::Application::GetInstance();
    auto &theme = GetTheme();

    // Create scene
    auto sceneId = GetRenderer().CreateScene();
    auto scene = std::make_shared<gui::SceneWidget>(
            *GetRenderer().GetScene(sceneId));
    impl_->scene = scene;
    scene->SetBackgroundColor(gui::Color(1.0, 1.0, 1.0));

    // Create light
    visualization::LightDescription lightDescription;
    lightDescription.intensity = 80000;
    lightDescription.direction = {-0.707, -.707, 0.0};
    lightDescription.customAttributes["custom_type"] = "SUN";

    impl_->lightSettings.hDirectionalLight =
            scene->GetScene()->AddLight(lightDescription);

    auto &lightSettings = impl_->lightSettings;
    std::string rsrcPath = app.GetResourcePath();
    auto iblPath = rsrcPath + "/default_ibl.ktx";
    lightSettings.hIbl =
            GetRenderer().AddIndirectLight(ResourceLoadRequest(iblPath.data()));
    scene->GetScene()->SetIndirectLight(lightSettings.hIbl);
    const auto kAmbientIntensity = 6000;
    scene->GetScene()->SetIndirectLightIntensity(kAmbientIntensity);

    auto skyPath = rsrcPath + "/default_sky.ktx";
    lightSettings.hSky =
            GetRenderer().AddSkybox(ResourceLoadRequest(skyPath.data()));
    scene->GetScene()->SetSkybox(lightSettings.hSky);

    SetGeometry(geometries);  // also updates the camera

    // Setup UI
    int spacing = std::max(1, int(std::ceil(0.25 * theme.fontSize)));

    auto buttonTop = std::make_shared<gui::Button>("Top");
    buttonTop->SetOnClicked([scene]() {
        scene->GoToCameraPreset(gui::SceneWidget::CameraPreset::PLUS_Y);
    });
    auto buttonFront = std::make_shared<gui::Button>("Front");
    buttonFront->SetOnClicked([scene]() {
        scene->GoToCameraPreset(gui::SceneWidget::CameraPreset::PLUS_Z);
    });
    auto buttonSide = std::make_shared<gui::Button>("Side");
    buttonSide->SetOnClicked([scene]() {
        scene->GoToCameraPreset(gui::SceneWidget::CameraPreset::PLUS_X);
    });
    auto bottomBar =
            std::make_shared<gui::Horiz>(spacing, gui::Margins(0, spacing));
    impl_->bottomBar = bottomBar;
    bottomBar->SetBackgroundColor(gui::Color(0, 0, 0, 0.5));
    bottomBar->AddChild(gui::Horiz::MakeStretch());
    bottomBar->AddChild(buttonTop);
    bottomBar->AddChild(buttonFront);
    bottomBar->AddChild(buttonSide);
    bottomBar->AddChild(gui::Horiz::MakeStretch());

    AddChild(scene);
    AddChild(bottomBar);

    auto renderScene = scene->GetScene();

    // Add light settings widget
    lightSettings.wgtBase = std::make_shared<gui::Vert>();
    lightSettings.wgtBase->SetFrame({0, 0, 250, 44});

    auto loadButtons =
            std::make_shared<gui::Horiz>(16, gui::Margins{4, 4, 4, 4});
    lightSettings.wgtLoadAmbient = std::make_shared<gui::Button>("Load ibl");
    lightSettings.wgtLoadAmbient->SetOnClicked([this, renderScene]() {
        auto dlg = std::make_shared<gui::FileDialog>(
                gui::FileDialog::Type::OPEN, "Open IBL", GetTheme());
        dlg->AddFilter(".ktx", "Khronos Texture (.ktx)");
        dlg->SetOnCancel([this]() { this->CloseDialog(); });
        dlg->SetOnDone([this, renderScene](const char *path) {
            this->CloseDialog();
            auto newIBL =
                    GetRenderer().AddIndirectLight(ResourceLoadRequest(path));
            if (newIBL) {
                impl_->lightSettings.hIbl = newIBL;

                auto intensity = renderScene->GetIndirectLightIntensity();

                renderScene->SetIndirectLight(newIBL);
                renderScene->SetIndirectLightIntensity(intensity);
            }
        });
        ShowDialog(dlg);
    });
    loadButtons->AddChild(lightSettings.wgtLoadAmbient);

    lightSettings.wgtLoadSky = std::make_shared<gui::Button>("Load skybox");
    lightSettings.wgtLoadSky->SetOnClicked([this, renderScene]() {
        auto dlg = std::make_shared<gui::FileDialog>(
                gui::FileDialog::Type::OPEN, "Open skybox", GetTheme());
        dlg->AddFilter(".ktx", "Khronos Texture (.ktx)");
        dlg->SetOnCancel([this]() { this->CloseDialog(); });
        dlg->SetOnDone([this, renderScene](const char *path) {
            this->CloseDialog();
            auto newSky = GetRenderer().AddSkybox(ResourceLoadRequest(path));
            if (newSky) {
                impl_->lightSettings.hSky = newSky;

                renderScene->SetSkybox(newSky);
            }
        });
        ShowDialog(dlg);
    });
    loadButtons->AddChild(lightSettings.wgtLoadSky);
    lightSettings.wgtBase->AddChild(loadButtons);

    lightSettings.wgtBase->AddChild(gui::Horiz::MakeFixed(8));
    lightSettings.wgtBase->AddChild(
            std::make_shared<gui::Label>("Light switches:"));
    auto checkboxes = std::make_shared<gui::Horiz>(0, gui::Margins{0, 0, 0, 8});
    lightSettings.wgtAmbientEnabled =
            std::make_shared<gui::Checkbox>("Ambient");
    lightSettings.wgtAmbientEnabled->SetChecked(true);
    lightSettings.wgtAmbientEnabled->SetOnChecked(
            [this, renderScene](bool checked) {
                if (checked) {
                    renderScene->SetIndirectLight(impl_->lightSettings.hIbl);
                } else {
                    renderScene->SetIndirectLight(IndirectLightHandle());
                }
            });
    checkboxes->AddChild(lightSettings.wgtAmbientEnabled);
    lightSettings.wgtSkyEnabled = std::make_shared<gui::Checkbox>("Sky");
    lightSettings.wgtSkyEnabled->SetChecked(true);
    lightSettings.wgtSkyEnabled->SetOnChecked(
            [this, renderScene](bool checked) {
                if (checked) {
                    renderScene->SetSkybox(impl_->lightSettings.hSky);
                } else {
                    renderScene->SetSkybox(SkyboxHandle());
                }
            });
    checkboxes->AddChild(lightSettings.wgtSkyEnabled);
    lightSettings.wgtDirectionalEnabled =
            std::make_shared<gui::Checkbox>("Sun");
    lightSettings.wgtDirectionalEnabled->SetChecked(true);
    lightSettings.wgtDirectionalEnabled->SetOnChecked(
            [this, renderScene](bool checked) {
                renderScene->SetEntityEnabled(
                        impl_->lightSettings.hDirectionalLight, checked);
            });
    checkboxes->AddChild(lightSettings.wgtDirectionalEnabled);
    lightSettings.wgtBase->AddChild(checkboxes);

    lightSettings.wgtIntensity = AddAttributeSlider(
            gui::Slider::INT, 0.0, 1000000.0, lightDescription.intensity,
            "Directional light intensity", lightSettings.wgtBase.get());
    lightSettings.wgtIntensity->OnValueChanged =
            [this, renderScene](double newValue) {
                renderScene->SetLightIntensity(
                        impl_->lightSettings.hDirectionalLight, newValue);
            };
    lightSettings.wgtAmbientIntensity = AddAttributeSlider(
            gui::Slider::INT, 0.0, 1000000.0, kAmbientIntensity,
            "Ambient light intensity", lightSettings.wgtBase.get());
    lightSettings.wgtAmbientIntensity->OnValueChanged =
            [renderScene](double newValue) {
                renderScene->SetIndirectLightIntensity(newValue);
            };
    lightSettings.wgtLightColor = std::make_shared<gui::ColorEdit>();
    lightSettings.wgtLightColor->SetValue({1, 1, 1});
    lightSettings.wgtLightColor->OnValueChanged =
            [this, renderScene](const gui::Color &newColor) {
                renderScene->SetLightColor(
                        impl_->lightSettings.hDirectionalLight,
                        {newColor.GetRed(), newColor.GetGreen(),
                         newColor.GetBlue()});
            };
    lightSettings.wgtBase->AddChild(lightSettings.wgtLightColor);
    auto lblLightColor =
            std::make_shared<gui::Label>("Directional light color");

    lightSettings.wgtBase->AddChild(lblLightColor);
    AddChild(lightSettings.wgtBase);
    lightSettings.wgtBase->SetVisible(false);

    // Create menu
    auto fileMenu = std::make_shared<gui::Menu>();
    fileMenu->AddItem("Open Geometry...", "Ctrl-O", FILE_OPEN);
    fileMenu->AddItem("Export RGB...", nullptr, FILE_EXPORT_RGB);
    fileMenu->SetEnabled(FILE_EXPORT_RGB, false);
    fileMenu->AddItem("Export depth image...", nullptr, FILE_EXPORT_DEPTH);
    fileMenu->SetEnabled(FILE_EXPORT_DEPTH, false);
    fileMenu->AddSeparator();
    fileMenu->AddItem("Close", "Ctrl-W", FILE_CLOSE);
    auto viewMenu = std::make_shared<gui::Menu>();
    viewMenu->AddItem("Points", nullptr, VIEW_POINTS);
    viewMenu->SetEnabled(VIEW_POINTS, false);
    viewMenu->AddItem("Wireframe", nullptr, VIEW_WIREFRAME);
    viewMenu->SetEnabled(VIEW_WIREFRAME, false);
    viewMenu->AddItem("Mesh", nullptr, VIEW_MESH);
    viewMenu->SetEnabled(VIEW_MESH, false);
    auto settingsMenu = std::make_shared<gui::Menu>();
    settingsMenu->AddItem("Default material", nullptr,
                          SETTINGS_DEFAULT_MATERIAL);
    settingsMenu->AddItem("Lightning", nullptr, SETTINGS_LIGHTNING);
    auto helpMenu = std::make_shared<gui::Menu>();
    helpMenu->AddItem("About", nullptr, HELP_ABOUT);
    helpMenu->AddItem("Contact", nullptr, HELP_CONTACT);
    auto menu = std::make_shared<gui::Menu>();
    menu->AddMenu("File", fileMenu);
    menu->AddMenu("View", viewMenu);
    menu->AddMenu("Settings", settingsMenu);
    menu->AddMenu("Help", helpMenu);
    this->SetMenubar(menu);
}

GuiVisualizer::~GuiVisualizer() {}

void GuiVisualizer::SetTitle(const std::string &title) {
    //    Super::SetTitle(title);
}

void GuiVisualizer::SetGeometry(
        const std::vector<std::shared_ptr<const geometry::Geometry>>
                &geometries) {
    auto *scene3d = impl_->scene->GetScene();
    for (auto &h : impl_->geometryHandles) {
        scene3d->RemoveGeometry(h);
    }
    impl_->geometryHandles.clear();

    geometry::AxisAlignedBoundingBox bounds;
    for (auto &g : geometries) {
        switch (g->GetGeometryType()) {
            case geometry::Geometry::GeometryType::OrientedBoundingBox:
            case geometry::Geometry::GeometryType::AxisAlignedBoundingBox:
            case geometry::Geometry::GeometryType::PointCloud:
            case geometry::Geometry::GeometryType::LineSet:
            case geometry::Geometry::GeometryType::MeshBase:
            case geometry::Geometry::GeometryType::TriangleMesh:
            case geometry::Geometry::GeometryType::HalfEdgeTriangleMesh:
            case geometry::Geometry::GeometryType::TetraMesh:
            case geometry::Geometry::GeometryType::Octree:
            case geometry::Geometry::GeometryType::VoxelGrid: {
                auto g3 =
                        std::static_pointer_cast<const geometry::Geometry3D>(g);
                bounds += g3->GetAxisAlignedBoundingBox();
                auto handle = scene3d->AddGeometry(*g3);

                impl_->geometryHandles.push_back(handle);
            }
            case geometry::Geometry::GeometryType::RGBDImage:
            case geometry::Geometry::GeometryType::Image:
            case geometry::Geometry::GeometryType::Unspecified:
                break;
        }
    }

    impl_->scene->SetupCamera(60.0, bounds, bounds.GetCenter().cast<float>());
}

void GuiVisualizer::Layout(const gui::Theme &theme) {
    auto r = GetContentRect();
    impl_->scene->SetFrame(r);

    auto bottomHeight = impl_->bottomBar->CalcPreferredSize(theme).height;
    gui::Rect bottomRect(0, r.GetBottom() - bottomHeight, r.width,
                         bottomHeight);
    impl_->bottomBar->SetFrame(bottomRect);

    const auto kLightSettignsWidth = 250;
    auto lightSettingsSize =
            impl_->lightSettings.wgtBase->CalcPreferredSize(theme);
    gui::Rect lightSettingsRect(r.width - kLightSettignsWidth, r.y,
                                kLightSettignsWidth, lightSettingsSize.height);
    impl_->lightSettings.wgtBase->SetFrame(lightSettingsRect);

    Super::Layout(theme);
}

bool GuiVisualizer::LoadGeometry(const std::string &path) {
    auto geometry = std::shared_ptr<geometry::Geometry3D>();

    auto mesh = std::make_shared<geometry::TriangleMesh>();
    bool meshSuccess = false;
    try {
        meshSuccess = io::ReadTriangleMesh(path, *mesh);
    } catch (...) {
        meshSuccess = false;
    }
    if (meshSuccess) {
        if (mesh->triangles_.size() == 0) {
            utility::LogWarning(
                    "Contains 0 triangles, will read as point cloud");
            mesh.reset();
        } else {
            mesh->ComputeVertexNormals();
            geometry = mesh;
        }
    } else {
        // LogError throws an exception, which we don't want, because this might
        // be a point cloud.
        utility::LogWarning("Failed to read %s", path.c_str());
        mesh.reset();
    }

    if (!geometry) {
        auto cloud = std::make_shared<geometry::PointCloud>();
        bool success = false;
        try {
            success = io::ReadPointCloud(path, *cloud);
        } catch (...) {
            success = false;
        }
        if (success) {
            utility::LogInfof("Successfully read %s", path.c_str());
            cloud->NormalizeNormals();
            geometry = cloud;
        } else {
            utility::LogWarning("Failed to read points %s", path.c_str());
            cloud.reset();
        }
    }

    if (geometry) {
        SetGeometry({geometry});
    }
    return (geometry != nullptr);
}

void GuiVisualizer::ExportRGB(const std::string &path) {
    ShowMessageBox("Not implemented", "ExportRGB() is not implemented yet");
}

void GuiVisualizer::ExportDepth(const std::string &path) {
    ShowMessageBox("Not implemented", "ExportDepth() is not implemented yet");
}

void GuiVisualizer::OnMenuItemSelected(gui::Menu::ItemId itemId) {
    auto menuId = MenuId(itemId);
    switch (menuId) {
        case FILE_OPEN: {
            auto dlg = std::make_shared<gui::FileDialog>(
                    gui::FileDialog::Type::OPEN, "Open Geometry", GetTheme());
            dlg->AddFilter(".ply .stl .obj .off .gltf .glb",
                           "Triangle mesh files (.ply, .stl, .obj, .off, "
                           ".gltf, .glb)");
            dlg->AddFilter(".xyz .xyzn .xyzrgb .ply .pcd .pts",
                           "Point cloud files (.xyz, .xyzn, .xyzrgb, .ply, "
                           ".pcd, .pts)");
            dlg->AddFilter(".ply", "Polygon files (.ply)");
            dlg->AddFilter(".stl", "Stereolithography files (.stl)");
            dlg->AddFilter(".obj", "Wavefront OBJ files (.obj)");
            dlg->AddFilter(".off", "Object file format (.off)");
            dlg->AddFilter(".gltf", "OpenGL transfer files (.gltf)");
            dlg->AddFilter(".glb", "OpenGL binary transfer files (.glb)");
            dlg->AddFilter(".xyz", "ASCII point cloud files (.xyz)");
            dlg->AddFilter(".xyzn", "ASCII point cloud with normals (.xyzn)");
            dlg->AddFilter(".xyzrgb",
                           "ASCII point cloud files with colors (.xyzrgb)");
            dlg->AddFilter(".pcd", "Point Cloud Data files (.pcd)");
            dlg->AddFilter(".pts", "3D Points files (.pts)");
            dlg->AddFilter("", "All files");
            dlg->SetOnCancel([this]() { this->CloseDialog(); });
            dlg->SetOnDone([this](const char *path) {
                this->CloseDialog();
                auto frame = this->GetFrame();
                auto title = std::string("Open3D - ") + path;
                std::vector<std::shared_ptr<const geometry::Geometry>> nothing;
                auto vis = std::make_shared<GuiVisualizer>(
                        nothing, title.c_str(), frame.width, frame.height,
                        frame.x + 20, frame.y + 20);
                gui::Application::GetInstance().AddWindow(vis);
                if (!vis->LoadGeometry(path)) {
                    auto err = std::string("Error reading geometry file '") +
                               path + "'";
                    vis->ShowMessageBox("Error loading geometry", err.c_str());
                }
            });
            ShowDialog(dlg);
            break;
        }
        case FILE_EXPORT_RGB:  // fall through
        case FILE_EXPORT_DEPTH: {
            auto dlg = std::make_shared<gui::FileDialog>(
                    gui::FileDialog::Type::SAVE, "Save File", GetTheme());
            dlg->AddFilter(".png", "PNG images (.png)");
            dlg->AddFilter("", "All files");
            dlg->SetOnCancel([this]() { this->CloseDialog(); });
            dlg->SetOnDone([this, menuId](const char *path) {
                this->CloseDialog();
                if (menuId == FILE_EXPORT_RGB) {
                    this->ExportRGB(path);
                } else {
                    this->ExportDepth(path);
                }
            });
            ShowDialog(dlg);
            break;
        }
        case FILE_CLOSE:
            this->Close();
            break;
        case VIEW_POINTS:
            break;
        case VIEW_WIREFRAME:
            break;
        case VIEW_MESH:
            break;
        case SETTINGS_DEFAULT_MATERIAL:
            break;
        case SETTINGS_LIGHTNING: {
            auto visibility = !impl_->lightSettings.wgtBase->IsVisible();
            impl_->lightSettings.wgtBase->SetVisible(visibility);
            break;
        }
        case HELP_ABOUT: {
            auto dlg = createAboutDialog(this);
            ShowDialog(dlg);
            break;
        }
        case HELP_CONTACT: {
            auto dlg = createContactDialog(this);
            ShowDialog(dlg);
            break;
        }
    }
}

}  // namespace visualization
}  // namespace open3d
