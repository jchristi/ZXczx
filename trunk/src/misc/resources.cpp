#include "resources.h"
#include "paths.h"
#include "../logs/logs.h"



namespace TA3D
{
namespace Resources
{


    //! Definition list of resources folders
    typedef std::vector<String> ResourcesFoldersList;

    //! List of resources folders
    static ResourcesFoldersList pResourcesFolders;


    # ifdef TA3D_PLATFORM_WINDOWS

    static void initForWindows()
    {
        AddSearchPath(Paths::ApplicationRoot + "resources\\");
        AddSearchPath(Paths::LocalData + "ta3d\\resources\\");
    }

    # else // ifdef TA3D_PLATFORM_WINDOWS

    # ifndef TA3D_PLATFORM_DARWIN
    static void initForDefaultUnixes()
    {
        String home = getenv("HOME");
        home += "/.ta3d/";

        AddSearchPath(home + "resources/");
        AddSearchPath("/usr/local/games/ta3d/");
        AddSearchPath("/usr/local/share/ta3d/");
        AddSearchPath("/opt/local/share/ta3d/");
        AddSearchPath(Paths::ApplicationRoot + "resources/");
    }

    # else // ifndef TA3D_PLATFORM_DARWIN

    static void initForDarwin()
    {
        String home = getenv("HOME");
        
        Paths::MakeDir(home + "/Library/Application Support/ta3d/");
        // Relative folder for the Application bundle
        AddSearchPath(Paths::ApplicationRoot + "../Resources/");
        AddSearchPath(home + "/Library/Application Support/ta3d/");
        // Unix compatibility
        AddSearchPath(home + "/.ta3d/resources/");
        // If using MacPorts
        AddSearchPath("/opt/local/share/ta3d/");
        AddSearchPath(Paths::ApplicationRoot + "resources/"); // TODO : Should be removed (need a fully working Application bundle)
    }

    # endif // ifndef TA3D_PLATFORM_DARWIN

    # endif // ifdef TA3D_PLATFORM_WINDOWS


    void Initialize()
    {
        # ifdef TA3D_PLATFORM_WINDOWS
        initForWindows();
        # else
        #   ifndef TA3D_PLATFORM_DARWIN
        initForDefaultUnixes();
        #   else
        initForDarwin();
        #   endif
        # endif
    }


    bool Find(const String& relFilename, String& out)
    {
        for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
        {
            out = *i;
            out += relFilename;
            if (Paths::Exists(out))
                return true;
        }
        return false;
    }

    bool AddSearchPath(const String& folder)
    {
        if (!folder.empty() && Paths::Exists(folder))
        {
            for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
            {
                if (folder == *i)
                    return false;
            }
            LOG_INFO("Folder: Resources: `" << folder << "`");
            pResourcesFolders.push_back(folder);
            return true;
        }
        return false;
    }


 
    bool Glob(std::vector<String>& out, const String& pattern, const bool emptyListBefore)
    {
        if (emptyListBefore)
            out.clear();
        for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
            Paths::Glob(out, *i + pattern, false);
        return !out.empty();
    }

    bool Glob(std::list<String>& out, const String& pattern, const bool emptyListBefore)
    {
        if (emptyListBefore)
            out.clear();
        for (ResourcesFoldersList::const_iterator i = pResourcesFolders.begin(); i != pResourcesFolders.end(); ++i)
            Paths::Glob(out, *i + pattern, false);
        return !out.empty();
    }




} // namespace Resources
} // namespace TA3D
