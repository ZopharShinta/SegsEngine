using Godot;
using System;
using System.IO;
using Godot.Collections;
using GodotTools.Internals;
using static GodotTools.Internals.Globals;
using File = GodotTools.Utils.File;
using Path = System.IO.Path;

namespace GodotTools
{
    public class BottomPanel : VBoxContainer
    {
        private EditorInterface editorInterface;

        private TabContainer panelTabs;

        private VBoxContainer panelBuildsTab;

        private ItemList buildTabsList;
        private TabContainer buildTabs;

        private Button warningsBtn;
        private Button errorsBtn;
        private Button viewLogBtn;

        private void _UpdateBuildTab(int index, int? currentTab)
        {
            var tab = (BuildTab)buildTabs.GetChild(index);

                string itemName = Path.GetFileNameWithoutExtension(tab.BuildInfo.Solution);
                itemName += " [" + tab.BuildInfo.Configuration + "]";

                buildTabsList.AddItem(itemName, tab.IconTexture);

                string itemTooltip = "Solution: " + tab.BuildInfo.Solution;
                itemTooltip += "\nConfiguration: " + tab.BuildInfo.Configuration;
                itemTooltip += "\nStatus: ";

                if (tab.BuildExited)
                    itemTooltip += tab.BuildResult == BuildTab.BuildResults.Success ? "Succeeded" : "Errored";
                else
                    itemTooltip += "Running";

                if (!tab.BuildExited || tab.BuildResult == BuildTab.BuildResults.Error)
                    itemTooltip += $"\nErrors: {tab.ErrorCount}";

                itemTooltip += $"\nWarnings: {tab.WarningCount}";

            buildTabsList.SetItemTooltip(index, itemTooltip);

            // If this tab was already selected before the changes or if no tab was selected
            if (currentTab == null || currentTab == index)
                {
                buildTabsList.Select(index);
                _BuildTabsItemSelected(index);
                }
            }
        private void _UpdateBuildTabsList()
        {
            buildTabsList.Clear();

            int? currentTab = buildTabs.CurrentTab;

            if (currentTab < 0 || currentTab >= buildTabs.GetTabCount())
                currentTab = null;

            for (int i = 0; i < buildTabs.GetChildCount(); i++)
                _UpdateBuildTab(i, currentTab);
        }

        public BuildTab GetBuildTabFor(BuildInfo buildInfo)
        {
            foreach (var buildTab in new Array<BuildTab>(buildTabs.GetChildren()))
            {
                if (buildTab.BuildInfo.Equals(buildInfo))
                    return buildTab;
            }

            var newBuildTab = new BuildTab(buildInfo);
            AddBuildTab(newBuildTab);

            return newBuildTab;
        }

        private void _BuildTabsItemSelected(int idx)
        {
            if (idx < 0 || idx >= buildTabs.GetTabCount())
                throw new IndexOutOfRangeException();

            buildTabs.CurrentTab = idx;
            if (!buildTabs.Visibility.Visible)
                buildTabs.Visibility.Visible = true;

            warningsBtn.Visibility.Visible = true;
            errorsBtn.Visibility.Visible = true;
            viewLogBtn.Visibility.Visible = true;
        }

        private void _BuildTabsNothingSelected()
        {
            if (buildTabs.GetTabCount() != 0)
            {
                // just in case
                buildTabs.Visibility.Visible = false;

                // This callback is called when clicking on the empty space of the list.
                // ItemList won't deselect the items automatically, so we must do it ourselves.
                buildTabsList.UnselectAll();
            }

            warningsBtn.Visibility.Visible = false;
            errorsBtn.Visibility.Visible = false;
            viewLogBtn.Visibility.Visible = false;
        }

        private void _WarningsToggled(bool pressed)
        {
            int currentTab = buildTabs.CurrentTab;

            if (currentTab < 0 || currentTab >= buildTabs.GetTabCount())
                throw new InvalidOperationException("No tab selected");

            var buildTab = (BuildTab)buildTabs.GetChild(currentTab);
            buildTab.WarningsVisible = pressed;
            buildTab.UpdateIssuesList();
        }

        private void _ErrorsToggled(bool pressed)
        {
            int currentTab = buildTabs.CurrentTab;

            if (currentTab < 0 || currentTab >= buildTabs.GetTabCount())
                throw new InvalidOperationException("No tab selected");

            var buildTab = (BuildTab)buildTabs.GetChild(currentTab);
            buildTab.ErrorsVisible = pressed;
            buildTab.UpdateIssuesList();
        }

        public void BuildProjectPressed()
        {
            if (!File.Exists(GodotSharpDirs.ProjectSlnPath))
                return; // No solution to build

            string editorScriptsMetadataPath = Path.Combine(GodotSharpDirs.ResMetadataDir, "scripts_metadata.editor");
            string playerScriptsMetadataPath = Path.Combine(GodotSharpDirs.ResMetadataDir, "scripts_metadata.editor_player");

            CsProjOperations.GenerateScriptsMetadata(GodotSharpDirs.ProjectCsProjPath, editorScriptsMetadataPath);

            if (File.Exists(editorScriptsMetadataPath))
            {
                try
                {
                    File.Copy(editorScriptsMetadataPath, playerScriptsMetadataPath);
                }
                catch (IOException e)
                {
                    GD.PushError($"Failed to copy scripts metadata file. Exception message: {e.Message}");
                    return;
                }
            }

            bool buildSuccess = BuildManager.BuildProjectBlocking("Debug");

            if (!buildSuccess)
                return;

            // Notify running game for hot-reload
            Internal.ScriptEditorDebuggerReloadScripts();

            // Hot-reload in the editor
            GodotSharpEditor.Instance.GetNode<HotReloadAssemblyWatcher>("HotReloadAssemblyWatcher").RestartTimer();

            if (Internal.IsAssembliesReloadingNeeded())
                Internal.ReloadAssemblies(softReload: false);
        }

        private void _ViewLogPressed()
        {
            if (!buildTabsList.IsAnythingSelected())
                return;

            var selectedItems = buildTabsList.GetSelectedItems();

            if (selectedItems.Length != 1)
                throw new InvalidOperationException($"Expected 1 selected item, got {selectedItems.Length}");

            int selectedItem = selectedItems[0];

            var buildTab = (BuildTab)buildTabs.GetTabControl(selectedItem);

            OS.ShellOpen(Path.Combine(buildTab.BuildInfo.LogsDirPath, BuildManager.MsBuildLogFileName));
        }

        public override void _Notification(int what)
        {
            base._Notification(what);

            if (what == EditorSettings.NotificationEditorSettingsChanged)
            {
                var editorBaseControl = editorInterface.GetBaseControl();
                panelTabs.AddStyleOverride("panel", editorBaseControl.GetStylebox("DebuggerPanel", "EditorStyles"));
                panelTabs.AddStyleOverride("tab_fg", editorBaseControl.GetStylebox("DebuggerTabFG", "EditorStyles"));
                panelTabs.AddStyleOverride("tab_bg", editorBaseControl.GetStylebox("DebuggerTabBG", "EditorStyles"));
            }
        }

        public void AddBuildTab(BuildTab buildTab)
        {
            buildTabs.AddChild(buildTab);
            RaiseBuildTab(buildTab);
        }

        public void RaiseBuildTab(BuildTab buildTab)
        {
            if (buildTab.GetParent() != buildTabs)
                throw new InvalidOperationException("Build tab is not in the tabs list");

            buildTabs.MoveChild(buildTab, 0);
            _UpdateBuildTabsList();
        }

        public void ShowBuildTab()
        {
            for (int i = 0; i < panelTabs.GetTabCount(); i++)
            {
                if (panelTabs.GetTabControl(i) == panelBuildsTab)
                {
                    panelTabs.CurrentTab = i;
                    GodotSharpEditor.Instance.MakeBottomPanelItemVisible(this);
                    return;
                }
            }

            GD.PushError("Builds tab not found");
        }

        public override void _Ready()
        {
            base._Ready();

            editorInterface = GodotSharpEditor.Instance.GetEditorInterface();

            var editorBaseControl = editorInterface.GetBaseControl();

            SizeFlags.Vertical = (int)SizeFlagsEnum.ExpandFill;
            SetAnchorsAndMarginsPreset(LayoutPreset.Wide);

            panelTabs = new TabContainer
            {
                TabAlign = TabContainer.TabAlignEnum.Left,
                Rect = {MinSize = new Vector2(0, 228) * EditorScale},
                SizeFlags = {Vertical=(int)SizeFlagsEnum.ExpandFill}
            };
            panelTabs.AddStyleOverride("panel", editorBaseControl.GetStylebox("DebuggerPanel", "EditorStyles"));
            panelTabs.AddStyleOverride("tab_fg", editorBaseControl.GetStylebox("DebuggerTabFG", "EditorStyles"));
            panelTabs.AddStyleOverride("tab_bg", editorBaseControl.GetStylebox("DebuggerTabBG", "EditorStyles"));
            AddChild(panelTabs);

            {
                // Builds tab
                panelBuildsTab = new VBoxContainer
                {
                    Name = "Builds".TTR(),
                    SizeFlags = {Horizontal= (int)SizeFlagsEnum.ExpandFill}
                };
                panelTabs.AddChild(panelBuildsTab);

                var toolBarHBox = new HBoxContainer { SizeFlags = {Horizontal = (int)SizeFlagsEnum.ExpandFill} };
                panelBuildsTab.AddChild(toolBarHBox);

                var buildProjectBtn = new Button
                {
                    Text = "Build Project".TTR(),
                    Focus= {Mode = FocusMode.None},
                };
                buildProjectBtn.PressedSignal += BuildProjectPressed;
                toolBarHBox.AddChild(buildProjectBtn);

                toolBarHBox.AddSpacer(begin: false);

                warningsBtn = new ToolButton
                {
                    Text = "Warnings".TTR(),
                    ToggleMode = true,
                    Pressed = true,
                    Visibility = { Visible = false },
                    Focus= {Mode = FocusMode.None},
                };
                warningsBtn.Toggled += _WarningsToggled;
                toolBarHBox.AddChild(warningsBtn);

                errorsBtn = new ToolButton
                {
                    Text = "Errors".TTR(),
                    ToggleMode = true,
                    Pressed = true,
                    Visibility = { Visible = false },
                    Focus= {Mode = FocusMode.None},
                };
                errorsBtn.Toggled += _ErrorsToggled;
                toolBarHBox.AddChild(errorsBtn);

                toolBarHBox.AddSpacer(begin: false);

                viewLogBtn = new Button
                {
                    Text = "View log".TTR(),
                    Focus= {Mode = FocusMode.None},
                    Visibility = { Visible = false }
                };
                viewLogBtn.PressedSignal += _ViewLogPressed;
                toolBarHBox.AddChild(viewLogBtn);

                var hsc = new HSplitContainer
                {
                    SizeFlags =
                    {
                        Horizontal= (int)SizeFlagsEnum.ExpandFill,
                        Vertical = (int)SizeFlagsEnum.ExpandFill
                    },
                };
                panelBuildsTab.AddChild(hsc);

                buildTabsList = new ItemList { SizeFlags = {Horizontal = (int)SizeFlagsEnum.ExpandFill} };
                buildTabsList.ItemSelected += _BuildTabsItemSelected;
                buildTabsList.NothingSelected += _BuildTabsNothingSelected;
                hsc.AddChild(buildTabsList);

                buildTabs = new TabContainer
                {
                    TabAlign = TabContainer.TabAlignEnum.Left,
                    SizeFlags = {Horizontal= (int)SizeFlagsEnum.ExpandFill},
                    TabsVisible = false
                };
                hsc.AddChild(buildTabs);
            }
        }
    }
}
