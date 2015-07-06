#ifndef SIMPLE_PLOT_MODULE_H
#define SIMPLE_PLOT_MODULE_H

#include "Framework.h"
#include "GLUtils.hpp"
#include <vector>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
//#include "VirtualTrackball.h"

#include "SurfacePlot.h"
#include "GraphPlot.h"
#include "VolumePlot.h"

class SimplePlot : public ocls::Module{
    
public:
    SimplePlot();

    virtual ~SimplePlot();
    
    /**
     * Called when the module is initialized by the framework
     */
    virtual void onInitialize(ocls::Framework* framework);
    /**
     * Called when the module is destroyed by the framework
     */
    virtual void onQuit(ocls::Framework* framework);
    
    /**
     * [Deprecated]
     * Called at the beginning of every simulation timestep
     */
    //virtual void onTimestepBegin(ocls::Framework* framework);
    
    /**
     * [Deprecated]
     * Called at the end of every simulation timestep
     */
    //virtual void onTimestepEnd(ocls::Framework* framework);

    /**
    * Create a new plot
    */
    Plot *createPlot(ocls::Data *data, ocls::Domain *domain, std::string name);

    /**
    * Update all plots
    */
    void updateAll();

    /**
    * Request repaint on all windows except the one with id
    * Note: Not thread safe!!
    */
    void requestRepaintAllExcept(size_t id);

private:
    ocls::RenderContext* m_render_context;
    std::vector<Plot*> m_plots;
};

#endif // SIMPLE_PLOT_MODULE_H
