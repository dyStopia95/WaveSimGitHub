#include "pch.h"
#include "ModelProcessor.h"
#include "Utility.h"
#include "UtilityWin32.h"

using namespace std;
using namespace std::filesystem;
using namespace std::string_literals;
using namespace ModelPipeline;
using namespace Library;

int main(int argc, char* argv[])
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		if (argc < 2)
		{
			throw exception("Usage: ModelPipeline.exe inputfilename");
		}

		string inputFile = argv[1];
		auto [inputFilename, inputDirectory] = Library::Utility::GetFileNameAndDirectory(inputFile);
		if (inputDirectory.empty())
		{
			inputDirectory = current_path().string();
		}

		current_path(inputDirectory);

		cout << "Reading: "s << inputFilename << endl;
		Model model = ModelProcessor::LoadModel(inputFilename, true);
		
		string outputFilename = inputFilename + ".bin"s;
		cout << "Writing: "s << outputFilename << endl;
		model.Save(outputFilename);
		cout << "Finished."s << endl;

	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
	}

	return 0;
}