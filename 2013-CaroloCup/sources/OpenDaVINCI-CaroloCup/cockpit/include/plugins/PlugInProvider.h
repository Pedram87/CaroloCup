/*
 * OpenDaVINCI.
 *
 * This software is open source. Please see COPYING and AUTHORS for further information.
 */

#ifndef COCKPIT_PLUGINS_PLUGINPROVIDER_H_
#define COCKPIT_PLUGINS_PLUGINPROVIDER_H_

#ifdef PANDABOARD
#include <stdc-predef.h>
#endif

#include <string>
#include <vector>
#include <map>

#include "QtIncludes.h"

// native.h must be included as first header file for definition of _WIN32_WINNT.
#include "core/native.h"

#include "core/SharedPointer.h"
#include "core/base/KeyValueConfiguration.h"
#include "core/base/Mutex.h"
#include "core/base/DataStoreManager.h"
#include "core/io/ContainerConference.h"

#include "plugins/PlugIn.h"

namespace cockpit {

    namespace plugins {

        using namespace std;

        /**
         * This class provides all available plugins.
         */
        class PlugInProvider : public QObject {
            private:
                /**
                 * "Forbidden" copy constructor. Goal: The compiler should warn
                 * already at compile time for unwanted bugs caused by any misuse
                 * of the copy constructor.
                 */
                PlugInProvider(const PlugInProvider &);
                /**
                 * "Forbidden" assignment operator. Goal: The compiler should warn
                 * already at compile time for unwanted bugs caused by any misuse
                 * of the assignment operator.
                 */
                PlugInProvider& operator=(const PlugInProvider &);

            private:
                /**
                 * Constructor.
                 *
                 * @param kvc KeyValueConfiguration for this GL-based widget.
                 * @param dsm DataStoreManager to be used for adding DataStores.
                 * @param conf Container conference to send data to.
                 * @param prnt Pointer to the container super window.
                 */
                PlugInProvider(const core::base::KeyValueConfiguration &kvc, core::base::DataStoreManager &dsm, core::io::ContainerConference &conf, QWidget *prnt);

            public:
                virtual ~PlugInProvider();

                /**
                 * This method returns a static instance for this factory.
                 *
                 * @param kvc KeyValueConfiguration for this widget.
                 * @param dsm DataStoreManager to be used for adding DataStores.
                 * @param conf Container conference to send data to.
                 * @param prnt Pointer to the container super window.
                 * @return Instance of this factory.
                 */
                static PlugInProvider& getInstance(const core::base::KeyValueConfiguration &kvc, core::base::DataStoreManager &dsm, core::io::ContainerConference &conf, QWidget *prnt);

                /**
                 * This method returns the list of available plugins.
                 *
                 * @return List of available plugins.
                 */
                const vector<string> getListOfAvailablePlugIns() const;

                /**
                 * This method returns the list of available master plugins.
                 *
                 * @return List of available master plugins.
                 */
                const vector<string> getListOfAvailableMasterPlugIns() const;

                /**
                 * This method returns the description for the given Plugin.
                 *
                 * @param pluginName Name of the Plugin.
                 * @return Description.
                 */
                string getDescriptionForPlugin(const string &pluginName);

                /**
                 * This method returns the plugin for the given name.
                 *
                 * @param name Name of the plugin.
                 * @return Plugin.
                 */
                core::SharedPointer<PlugIn> getPlugIn(const string &name);

            private:
                static core::base::Mutex m_singletonMutex;
                static PlugInProvider* m_singleton;

                vector<string> m_listOfAvailablePlugIns;
                map<string,string> m_listOfDescriptions;
                core::base::KeyValueConfiguration m_kvc;
                core::base::DataStoreManager &m_dataStoreManager;
                core::io::ContainerConference &m_conference;
                QWidget *m_parent;

                /**
                 * This method creates a new instance for the given name.
                 *
                 * @param name Name of the plugin.
                 * @return New instance.
                 */
                core::SharedPointer<PlugIn> createPlugIn(const string &name);
        };

    } // plugins

} // cockpit

#endif /*COCKPIT_PLUGINS_PLUGINPROVIDER_H_*/

