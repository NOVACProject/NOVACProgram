#pragma once

#include "../Common/Common.h"
#include "../Common/XMLFileReader.h"
#include <SpectralEvaluation/Evaluation/FitWindow.h>

namespace FileHandler {
/** A <b>CFitWindowFileHandler</b> object is capable of reading
    and writing fit windows from a .nfw (NovacFitWindow) file. */

	class CFitWindowFileHandler :	public CXMLFileReader
	{
	public:
		CFitWindowFileHandler(void);
		~CFitWindowFileHandler(void);

		/** Reads the desired fit window from the file.
				@param window - will be filled with the read fit-window settings if successfull
				@param fileName - the name and path of the file to read from
				@param index - the zero-based index of the fit window to read (each .nfw file
					can contain several fit-windows)
				@return SUCCESS on success */
		RETURN_CODE ReadFitWindow(Evaluation::CFitWindow &window, const CString &fileName, int index = 0);

		/** Writes the supplied fit-window to a file.
				@param window - the fit window to be written to file
				@param fileName - the name and path of the file to which to write
				@param overWrite - if true the file will be overwritten, if false, the file will be appended */
		RETURN_CODE WriteFitWindow(const Evaluation::CFitWindow &window, const CString &fileName, bool overWrite);

	protected:

		/** Parses a fit-window section of the .nfs file */
		RETURN_CODE Parse_FitWindow(Evaluation::CFitWindow &window);

		/** Parses a reference-file section of the .nfs file */
		RETURN_CODE Parse_Reference(Evaluation::CFitWindow &window);
	};
}