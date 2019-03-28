NOVAC Program Release Notes
-----------------------------------------------------
Version 3.1 (TBD)

Bug fixes
* Set time limit on how much time to spend downloading pak files from one spectrometer (#43)
* Fix issue with flux data with different UTC date plotted as if it would have the same date (#56)

New features
* Option to show columns for current day plot to support fixed view measurements(#53)
* Addition of column history tab (#57)
* Show last 24 hours of data instead of just current UTC day on main tab and overview tab (#70, #78)
* Allow configurable FTP timeout to instrument (#76)
* FTP upload server login information is now stored in a separate ftplogin.xml file
* Status Message area resizes when window is resized
* Open application with size maximized

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



