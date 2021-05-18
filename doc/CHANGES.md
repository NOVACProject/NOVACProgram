NOVAC Program Release Notes

-----------------------------------------------------
Version 3.3 (TBD)

Bug fixes
* Fix quirks when deleting scanners (#140)
* Fix evaluation progress bar not working in Re-Evaluation dialog (#132)
* Return parsing error if no ws, wd, or ph read from wind file in post flux dialog (#147)

New features
* Directory polling option (#111)
* Merge Evaluation Log feature (#136)
* Upload evaluation logs to FTP/SFTP server (#137)
* Use passive FTP when connecting to instrument PC

-----------------------------------------------------
Version 3.2 (January 2021)

Bug fixes
* Fix issue with columns from same scan initially plotting at same time in history (#99)
* Fix issue with software crashing on re-evaluation when solar spectrum file is used (#103)
* Fix peak intensity/specsaturation (#108)
* Fix issue with SO2 blue reference line being flat during re-evaluation

New features
* Support for user-configurable spectrometer (#6)
* SFTP support for uploading pak files to offsite server (#58)
* Hostname support for FTP to instrument computer (#81)
* Addition of flux history tab (#107)
* AveSpec spectrometer support
* Axiomtec instrument computer support
* Add new "IntegrationMethod" property to STD file (#126)
* Streamlined logging for FTP upload

-----------------------------------------------------

Version 3.1 (June 2019)

Bug fixes
* Set time limit on how much time to spend downloading pak files from one spectrometer (#43)
* Fix issue with flux data with different UTC date plotted as if it would have the same date (#56)
* Fix duplicate spectral data entries when 'Find Optimal Shift' is checked in ReEvaluation dialog Fit Window tab (#89)

New features
* Option to show columns for current day plot to support fixed view measurements(#53)
* Addition of column history tab (#57)
* Show last 24 hours of data instead of just current UTC day on main tab, overview tab, and instrument tab (#70, #78, #82)
* Allow configurable FTP timeout to instrument (#76)
* FTP upload server login information is now stored in a separate ftplogin.xml file
* Status Message area resizes when window is resized
* Open application with size maximized

Other
* Refactoring and separation of shared spectral evaluation code into its own project.

-----------------------------------------------------
Version 3.0 (February 2018)

Bug fixes
* Pak-files only download once after start-up (#2)
* Broken sleep option (#14)
* Check marks do not appear in View menu (#18)
* Prevent program crashing during re-evaluation (#19)
* Hitting Browse Files menu causes program to crash (#20)
* Re-evaluation scan browse files button does not open all files (#25)
* Removing one spectrometer removes all spectrometer under a volcano (#30)

New features
* Implement user-defined observatory name (#5)
* Make electronics tag default to 1 and also make it configurable (#7)
* Add 'MAYAPRO' to spectrometer model drop down (#8)
* Add 'Other' as an option at initial scanner configuration (#9)
* Add 'View' option to evaluation tabs (#12)
* Make FTP default in communications tab (#13)
* Add a check box to activate wind file options (#17)

Removal of unused features
* 'Instrument' option (#10)
* 'Motor' configuration (#11)
* 'Dark' tab from configuration dialog (#15)
* 'Remote Configuration' tab (#16)
* Summarize Flux Data menu item (#32)



