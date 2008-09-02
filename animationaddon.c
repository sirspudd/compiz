/**
 * Animation plugin for compiz/beryl
 *
 * animation.c
 *
 * Copyright : (C) 2006 Erkin Bahceci
 * E-mail    : erkinbah@gmail.com
 *
 * Based on Wobbly and Minimize plugins by
 *           : David Reveman
 * E-mail    : davidr@novell.com>
 *
 * Airplane added by : Carlo Palma
 * E-mail            : carlopalma@salug.it
 * Based on code originally written by Mark J. Kilgard
 *
 * Beam-Up added by : Florencio Guimaraes
 * E-mail           : florencio@nexcorp.com.br
 *
 * Fold and Skewer added by : Tomasz Kolodziejski
 * E-mail                   : tkolodziejski@gmail.com
 *
 * Hexagon tessellator added by : Mike Slegeir
 * E-mail                       : mikeslegeir@mail.utexas.edu>
 *
 * Particle system added by : (C) 2006 Dennis Kasprzyk
 * E-mail                   : onestone@beryl-project.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

/*
 * TODO:
 *
 * - Custom bounding box update function for Airplane
 *
 * - Proper side surface normals for lighting
 * - decoration shadows
 *   - shadow quad generation
 *   - shadow texture coords (from clip tex. matrices)
 *   - draw shadows
 *   - fade in shadows
 *
 * - Voronoi tessellation
 * - Brick tessellation
 * - Triangle tessellation
 * - Hexagonal tessellation
 *
 * Effects:
 * - Circular action for tornado type fx
 * - Tornado 3D (especially for minimize)
 * - Helix 3D (hor. strips descend while they rotate and fade in)
 * - Glass breaking 3D
 *   - Gaussian distr. points (for gradually increasing polygon size
 *                           starting from center or near mouse pointer)
 *   - Drawing cracks
 *   - Gradual cracking
 *
 * - fix slowness during transparent cube with <100 opacity
 * - fix occasional wrong side color in some windows
 * - fix on top windows and panels
 *   (These two only matter for viewing during Rotate Cube.
 *    All windows should be painted with depth test on
 *    like 3d-plugin does)
 * - play better with rotate (fix cube face drawn on top of polygons
 *   after 45 deg. rotation)
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <GL/glu.h>
#include "animationaddon.h"

int animDisplayPrivateIndex;
int animAddonFunctionsPrivateIndex;
CompMetadata animMetadata;


AnimEffect animEffects[NUM_EFFECTS];

ExtensionPluginInfo animExtensionPluginInfo = {
    .nEffects		= NUM_EFFECTS,
    .effects		= animEffects,

    .nEffectOptions	= ANIMADDON_SCREEN_OPTION_NUM,

    .prePaintOutputFunc	= polygonsPrePaintOutput
};

OPTION_GETTERS (GET_ANIMADDON_DISPLAY(w->screen->display)->animBaseFunctions,
		&animExtensionPluginInfo, NUM_NONEFFECT_OPTIONS)

static Bool
animSetScreenOptions(CompPlugin *plugin,
		     CompScreen * screen,
		     const char *name,
		     CompOptionValue * value)
{
    CompOption *o;
    int index;

    ANIMADDON_SCREEN (screen);

    o = compFindOption(as->opt, NUM_OPTIONS(as), name, &index);
    if (!o)
	return FALSE;

    switch (index)
    {
    default:
	return compSetScreenOption (screen, o, value);
	break;
    }

    return FALSE;
}

static const CompMetadataOptionInfo animAddonDisplayOptionInfo[] = {
    { "abi", "int", 0, 0, 0 },
    { "index", "int", 0, 0, 0 }
};

static CompOption *
animGetDisplayOptions (CompPlugin  *plugin,
		       CompDisplay *display,
		       int         *count)
{
    ANIMADDON_DISPLAY (display);
    *count = NUM_OPTIONS (ad);
    return ad->opt;
}

static Bool
animSetDisplayOption (CompPlugin      *plugin,
		      CompDisplay     *display,
		      const char      *name,
		      CompOptionValue *value)
{
    CompOption      *o;
    int	            index;
    ANIMADDON_DISPLAY (display);
    o = compFindOption (ad->opt, NUM_OPTIONS (ad), name, &index);
    if (!o)
	return FALSE;

    switch (index) {
    case ANIMADDON_DISPLAY_OPTION_ABI:
    case ANIMADDON_DISPLAY_OPTION_INDEX:
        break;
    default:
        return compSetDisplayOption (display, o, value);
    }

    return FALSE;
}

static AnimWindowEngineData *
getAnimWindowEngineData (CompWindow *w)
{
    ANIMADDON_WINDOW (w);

    return &aw->eng;
}

int
getIntenseTimeStep (CompScreen *s)
{
    ANIMADDON_SCREEN (s);

    return as->opt[ANIMADDON_SCREEN_OPTION_TIME_STEP_INTENSE].value.i;
}

AnimAddonFunctions animAddonFunctions =
{
    .getAnimWindowEngineData		= getAnimWindowEngineData,
    .getIntenseTimeStep			= getIntenseTimeStep,

    .initParticles			= initParticles,
    .finiParticles			= finiParticles,
    .drawParticleSystems		= drawParticleSystems,
    .particlesUpdateBB			= particlesUpdateBB,
    .particlesCleanup			= particlesCleanup,
    .particlesPrePrepPaintScreen	= particlesPrePrepPaintScreen,

    .polygonsAnimInit			= polygonsAnimInit,
    .polygonsAnimStep			= polygonsAnimStep,
    .polygonsPrePaintWindow		= polygonsPrePaintWindow,
    .polygonsPostPaintWindow		= polygonsPostPaintWindow,
    .polygonsStoreClips			= polygonsStoreClips,
    .polygonsDrawCustomGeometry		= polygonsDrawCustomGeometry,
    .polygonsUpdateBB			= polygonsUpdateBB,
    .polygonsPrePreparePaintScreen	= polygonsPrePreparePaintScreen,
    .polygonsCleanup			= polygonsCleanup,
    .polygonsRefresh			= polygonsRefresh,
    .polygonsDeceleratingAnimStepPolygon= polygonsDeceleratingAnimStepPolygon,
    .freePolygonObjects			= freePolygonObjects,
    .tessellateIntoRectangles		= tessellateIntoRectangles,
    .tessellateIntoHexagons		= tessellateIntoHexagons
};

static const CompMetadataOptionInfo animAddonScreenOptionInfo[] = {
    // Misc. settings
    { "time_step_intense", "int", "<min>1</min>", 0, 0 },
    // Effect settings
    { "helix_num_twists", "int", "<min>1</min>", 0, 0 },
    { "helix_gridy", "int", "<min>5</min>", 0, 0 },
    { "helix_thickness", "float", 0, 0, 0 },
    { "helix_direction", "bool", 0, 0, 0 },
    { "helix_spin_direction", "int", "<min>0</min>", 0, 0 },

};

static CompOption *
animGetScreenOptions(CompPlugin *plugin, CompScreen * screen, int *count)
{
    ANIMADDON_SCREEN (screen);

    *count = NUM_OPTIONS(as);
    return as->opt;
}

AnimEffect AnimEffectHelix	= &(AnimEffectInfo) {};

static void
initEffectProperties (AnimAddonDisplay *ad)
{
    memcpy ((AnimEffectInfo *)AnimEffectHelix, &(AnimEffectInfo)
	{"animationaddon:Helix",
	 {TRUE, TRUE, TRUE, FALSE, FALSE},
	 {.prePaintWindowFunc		= polygonsPrePaintWindow,
	  .postPaintWindowFunc		= polygonsPostPaintWindow,
	  .animStepFunc			= polygonsAnimStep,
	  .initFunc			= fxHelixInit,
	  .addCustomGeometryFunc	= polygonsStoreClips,
	  .drawCustomGeometryFunc	= polygonsDrawCustomGeometry,
	  .updateBBFunc			= ad->animBaseFunctions->updateBBScreen,
	  .prePrepPaintScreenFunc	= polygonsPrePreparePaintScreen,
	  .cleanupFunc			= polygonsCleanup,
	  .refreshFunc			= polygonsRefresh}},
	  sizeof (AnimEffectInfo));

    AnimEffect animEffectsTmp[NUM_EFFECTS] =
    {
	AnimEffectHelix
    };
    memcpy (animEffects,
	    animEffectsTmp,
	    NUM_EFFECTS * sizeof (AnimEffect));
}

static Bool animInitDisplay(CompPlugin * p, CompDisplay * d)
{
    AnimAddonDisplay *ad;
    int animFunctionIndex;

    if (!checkPluginABI ("core", CORE_ABIVERSION) ||
        !checkPluginABI ("animation", ANIMATION_ABIVERSION))
	return FALSE;

    if (!getPluginDisplayIndex (d, "animation", &animFunctionIndex))
	return FALSE;

    ad = calloc(1, sizeof(AnimAddonDisplay));
    if (!ad)
	return FALSE;

    if (!compInitDisplayOptionsFromMetadata (d,
					     &animMetadata,
					     animAddonDisplayOptionInfo,
					     ad->opt,
					     ANIMADDON_DISPLAY_OPTION_NUM))
    {
	free (ad);
	return FALSE;
    }

    ad->screenPrivateIndex = allocateScreenPrivateIndex(d);
    if (ad->screenPrivateIndex < 0)
    {
	free(ad);
	return FALSE;
    }

    ad->animBaseFunctions = d->base.privates[animFunctionIndex].ptr;

    initEffectProperties (ad);

    ad->opt[ANIMADDON_DISPLAY_OPTION_ABI].value.i   = ANIMATIONADDON_ABIVERSION;
    ad->opt[ANIMADDON_DISPLAY_OPTION_INDEX].value.i = animAddonFunctionsPrivateIndex;

    d->base.privates[animDisplayPrivateIndex].ptr = ad;
    d->base.privates[animAddonFunctionsPrivateIndex].ptr = &animAddonFunctions;

    return TRUE;
}

static void animFiniDisplay(CompPlugin * p, CompDisplay * d)
{
    ANIMADDON_DISPLAY (d);

    freeScreenPrivateIndex(d, ad->screenPrivateIndex);

    free(ad);
}

static Bool animInitScreen(CompPlugin * p, CompScreen * s)
{
    AnimAddonScreen *as;

    ANIMADDON_DISPLAY (s->display);

    as = calloc(1, sizeof(AnimAddonScreen));
    if (!as)
	return FALSE;

    if (!compInitScreenOptionsFromMetadata (s,
					    &animMetadata,
					    animAddonScreenOptionInfo,
					    as->opt,
					    ANIMADDON_SCREEN_OPTION_NUM))
    {
	free (as);
	return FALSE;
    }

    as->windowPrivateIndex = allocateWindowPrivateIndex(s);
    if (as->windowPrivateIndex < 0)
    {
	compFiniScreenOptions (s, as->opt, ANIMADDON_SCREEN_OPTION_NUM);
	free(as);
	return FALSE;
    }

    as->output = &s->fullscreenOutput;

    animExtensionPluginInfo.effectOptions = &as->opt[NUM_NONEFFECT_OPTIONS];

    ad->animBaseFunctions->addExtension (s, &animExtensionPluginInfo);

    s->base.privates[ad->screenPrivateIndex].ptr = as;

    return TRUE;
}

static void animFiniScreen(CompPlugin * p, CompScreen * s)
{
    ANIMADDON_SCREEN (s);
    ANIMADDON_DISPLAY (s->display);

    ad->animBaseFunctions->removeExtension (s, &animExtensionPluginInfo);

    freeWindowPrivateIndex(s, as->windowPrivateIndex);

    compFiniScreenOptions (s, as->opt, ANIMADDON_SCREEN_OPTION_NUM);

    free(as);
}

static Bool animInitWindow(CompPlugin * p, CompWindow * w)
{
    CompScreen *s = w->screen;
    AnimAddonWindow *aw;

    ANIMADDON_DISPLAY (s->display);
    ANIMADDON_SCREEN (s);

    aw = calloc(1, sizeof(AnimAddonWindow));
    if (!aw)
	return FALSE;

    aw->eng.polygonSet = NULL;

    w->base.privates[as->windowPrivateIndex].ptr = aw;

    aw->com = ad->animBaseFunctions->getAnimWindowCommon (w);

    return TRUE;
}

static void animFiniWindow(CompPlugin * p, CompWindow * w)
{
    ANIMADDON_WINDOW (w);

    free(aw);
}

static CompBool
animInitObject (CompPlugin *p,
		CompObject *o)
{
    static InitPluginObjectProc dispTab[] = {
	(InitPluginObjectProc) 0, /* InitCore */
	(InitPluginObjectProc) animInitDisplay,
	(InitPluginObjectProc) animInitScreen,
	(InitPluginObjectProc) animInitWindow
    };

    RETURN_DISPATCH (o, dispTab, ARRAY_SIZE (dispTab), TRUE, (p, o));
}

static void
animFiniObject (CompPlugin *p,
		CompObject *o)
{
    static FiniPluginObjectProc dispTab[] = {
	(FiniPluginObjectProc) 0, /* FiniCore */
	(FiniPluginObjectProc) animFiniDisplay,
	(FiniPluginObjectProc) animFiniScreen,
	(FiniPluginObjectProc) animFiniWindow
    };

    DISPATCH (o, dispTab, ARRAY_SIZE (dispTab), (p, o));
}

static CompOption *
animGetObjectOptions (CompPlugin *plugin,
		      CompObject *object,
		      int	   *count)
{
    static GetPluginObjectOptionsProc dispTab[] = {
	(GetPluginObjectOptionsProc) 0, /* GetCoreOptions */
	(GetPluginObjectOptionsProc) animGetDisplayOptions,
	(GetPluginObjectOptionsProc) animGetScreenOptions
    };

    RETURN_DISPATCH (object, dispTab, ARRAY_SIZE (dispTab),
		     (void *) (*count = 0), (plugin, object, count));
}

static CompBool
animSetObjectOption (CompPlugin      *plugin,
		     CompObject      *object,
		     const char      *name,
		     CompOptionValue *value)
{
    static SetPluginObjectOptionProc dispTab[] = {
	(SetPluginObjectOptionProc) 0, /* SetCoreOption */
	(SetPluginObjectOptionProc) animSetDisplayOption,
	(SetPluginObjectOptionProc) animSetScreenOptions
    };

    RETURN_DISPATCH (object, dispTab, ARRAY_SIZE (dispTab), FALSE,
		     (plugin, object, name, value));
}

static Bool animInit(CompPlugin * p)
{
    if (!compInitPluginMetadataFromInfo (&animMetadata,
					 p->vTable->name,
					 0, 0,
					 animAddonScreenOptionInfo,
					 ANIMADDON_SCREEN_OPTION_NUM))
	return FALSE;

    animDisplayPrivateIndex = allocateDisplayPrivateIndex();
    if (animDisplayPrivateIndex < 0)
    {
	compFiniMetadata (&animMetadata);
	return FALSE;
    }

    animAddonFunctionsPrivateIndex = allocateDisplayPrivateIndex ();
    if (animAddonFunctionsPrivateIndex < 0)
    {
	freeDisplayPrivateIndex (animDisplayPrivateIndex);
	compFiniMetadata (&animMetadata);
	return FALSE;
    }

    compAddMetadataFromFile (&animMetadata, p->vTable->name);

    return TRUE;
}

static void animFini(CompPlugin * p)
{
    freeDisplayPrivateIndex(animDisplayPrivateIndex);
    freeDisplayPrivateIndex (animAddonFunctionsPrivateIndex);
    compFiniMetadata (&animMetadata);
}

static CompMetadata *
animGetMetadata (CompPlugin *plugin)
{
    return &animMetadata;
}

CompPluginVTable animVTable = {
    "animationaddon",
    animGetMetadata,
    animInit,
    animFini,
    animInitObject,
    animFiniObject,
    animGetObjectOptions,
    animSetObjectOption,
};

CompPluginVTable*
getCompPluginInfo20070830 (void)
{
    return &animVTable;
}

