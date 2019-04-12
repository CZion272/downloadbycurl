{
  "targets": [ 
	{
	  "target_name": "<(module_name)",
		"sources": [
		"init.cc",
		"moc_download.cpp",
		"download.cpp",
		"download.h",
		"moc_task.cpp",
		"task.h",
		"task.cpp"
		],
		"Release": {
		"configurations": {
				"VCCLCompilerTool": {
					"RuntimeLibrary": "2",
				},
			},
		},
		"defines": [
			"QT_DEPRECATED_WARNINGS",
			"QT_NO_DEBUG",
			"QT_GUI_LIB",
			"QT_CORE_LIB",
			"QT_WIDGETS_LIB"
			],			
	  "include_dirs": [
		 "libcurl/include",
		 "$(QTDIR)/include",
		 "$(QTDIR)/include/QtGui",
		 "$(QTDIR)/include/QtANGLE",
		 "$(QTDIR)/include/QtCore",
		 "$(QTDIR)/include/QtWidgets"
		 ],
	  "link_settings": {
		 "libraries": [
		 "../libcurl/lib/libcurl.lib",
		 "$(QTDIR)/lib/Qt5Gui.lib",		 
		 "$(QTDIR)/lib/Qt5Core.lib",
		 "$(QTDIR)/lib/Qt5Widgets.lib"
		 ]
		 }
	},    
	{
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ 
				"<(module_name)"
				 ],	  
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
  }
	]
}
