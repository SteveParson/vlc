/*****************************************************************************
 * gtk_open.c : functions to handle file/disc/network open widgets.
 *****************************************************************************
 * Copyright (C) 2000, 2001 VideoLAN
 * $Id: gtk_open.c,v 1.28 2002/06/07 14:30:41 sam Exp $
 *
 * Authors: Samuel Hocevar <sam@zoy.org>
 *          St�phane Borel <stef@via.ecp.fr>
 *      
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include <sys/types.h>                                              /* off_t */
#include <stdlib.h>

#include <vlc/vlc.h>
#include <vlc/intf.h>

#ifdef MODULE_NAME_IS_gnome
#   include <gnome.h>
#else
#   include <gtk/gtk.h>
#endif

#include <string.h>

#include "gtk_callbacks.h"
#include "gtk_interface.h"
#include "gtk_support.h"
#include "gtk_playlist.h"
#include "gtk_common.h"

#include "netutils.h"

/*****************************************************************************
 * Fileopen callbacks
 *****************************************************************************
 * The following callbacks are related to the file requester.
 *****************************************************************************/
gboolean GtkFileOpenShow( GtkWidget       *widget,
                          gpointer         user_data )
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(widget), (char*)user_data );

    /* If we have never used the file selector, open it */
    if( !GTK_IS_WIDGET( p_intf->p_sys->p_fileopen ) )
    {
        char *psz_path;

        p_intf->p_sys->p_fileopen = create_intf_fileopen();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_fileopen ),
                             "p_intf", p_intf );

        if( (psz_path = config_GetPsz( p_intf, "search-path" )) )
            gtk_file_selection_set_filename( GTK_FILE_SELECTION(
                p_intf->p_sys->p_fileopen ), psz_path );
        if( psz_path ) free( psz_path );
    }

    gtk_widget_show( p_intf->p_sys->p_fileopen );
    gdk_window_raise( p_intf->p_sys->p_fileopen->window );

    return TRUE;
}


void GtkFileOpenCancel( GtkButton * button, gpointer user_data )
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}

void GtkFileOpenOk( GtkButton * button, gpointer user_data )
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_fileopen" );
    playlist_t *    p_playlist;
    GtkCList *      p_playlist_clist;
    GtkWidget *     p_filesel;
    gchar *         psz_filename;

    p_playlist = vlc_object_find( p_intf, VLC_OBJECT_PLAYLIST, FIND_ANYWHERE );
    if( p_playlist == NULL )
    {
        return;
    }

    /* hide the file selector */
    p_filesel = gtk_widget_get_toplevel( GTK_WIDGET(button) );
    gtk_widget_hide( p_filesel );

    /* add the new file to the interface playlist */
    psz_filename =
        gtk_file_selection_get_filename( GTK_FILE_SELECTION( p_filesel ) );
    playlist_Add( p_playlist, (char*)psz_filename,
                  PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );

    /* catch the GTK CList */
    p_playlist_clist = GTK_CLIST( gtk_object_get_data(
        GTK_OBJECT( p_intf->p_sys->p_playwin ), "playlist_clist" ) );
    /* update the plugin display */
    GtkRebuildCList( p_playlist_clist, p_playlist );

    vlc_object_release( p_playlist );
}

/*****************************************************************************
 * Open disc callbacks
 *****************************************************************************
 * The following callbacks are related to the disc manager.
 *****************************************************************************/
gboolean GtkDiscOpenShow( GtkWidget       *widget,
                          gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(widget), (char*)user_data );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_disc ) )
    {
        p_intf->p_sys->p_disc = create_intf_disc();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_disc ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_disc );
    gdk_window_raise( p_intf->p_sys->p_disc->window );

    return TRUE;
}


void GtkDiscOpenDvd( GtkToggleButton * togglebutton, gpointer user_data )
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(togglebutton), "intf_disc" );

    if( togglebutton->active )
    {
        char *psz_dvd_device;

        if( (psz_dvd_device = config_GetPsz( p_intf, "dvd" )) )
            gtk_entry_set_text(
                GTK_ENTRY( lookup_widget( GTK_WIDGET(togglebutton),
                                          "disc_name" ) ), psz_dvd_device );
        if( psz_dvd_device ) free( psz_dvd_device );
    }
}

void GtkDiscOpenVcd( GtkToggleButton * togglebutton, gpointer user_data )
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(togglebutton), "intf_disc" );

    if( togglebutton->active )
    {
        char *psz_vcd_device;

        if( (psz_vcd_device = config_GetPsz( p_intf, "vcd" )) )
            gtk_entry_set_text(
                GTK_ENTRY( lookup_widget( GTK_WIDGET(togglebutton),
                                          "disc_name" ) ), psz_vcd_device );
        if( psz_vcd_device ) free( psz_vcd_device );
    }
}

void GtkDiscOpenOk( GtkButton * button, gpointer user_data )
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_disc" );
    playlist_t *    p_playlist;
    GtkCList *      p_playlist_clist;
    char *          psz_device, *psz_source, *psz_method;
    int             i_title, i_chapter;

    p_playlist = vlc_object_find( p_intf, VLC_OBJECT_PLAYLIST, FIND_ANYWHERE );
    if( p_playlist == NULL )
    {
        return;
    }

    gtk_widget_hide( p_intf->p_sys->p_disc );
    psz_device = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                                         GTK_WIDGET(button), "disc_name" ) ) );

    /* Check which method was activated */
    if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                          "disc_dvd" ) )->active )
    {
        psz_method = "dvd";
    }
    else if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                               "disc_vcd" ) )->active )
    {
        psz_method = "vcd";
    }
    else
    {
        msg_Err( p_intf, "unknown disc type toggle button position" );
        return;
    }
    
    /* Select title and chapter */
    i_title = gtk_spin_button_get_value_as_int(
                              GTK_SPIN_BUTTON( lookup_widget(
                                  GTK_WIDGET(button), "disc_title" ) ) );

    i_chapter = gtk_spin_button_get_value_as_int(
                              GTK_SPIN_BUTTON( lookup_widget(
                                  GTK_WIDGET(button), "disc_chapter" ) ) );
    
    /* "dvd:foo" has size 5 + strlen(foo) */
    psz_source = malloc( 3 /* "dvd" */ + 1 /* ":" */
                           + strlen( psz_device ) + 2 /* @, */
                           + 4 /* i_title & i_chapter < 100 */ + 1 /* "\0" */ );
    if( psz_source == NULL )
    {
        return;
    }

    /* Build source name and add it to playlist */
    sprintf( psz_source, "%s:%s@%d,%d",
             psz_method, psz_device, i_title, i_chapter );
    playlist_Add( p_playlist, psz_source,
                  PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
    free( psz_source );

    /* catch the GTK CList */
    p_playlist_clist = GTK_CLIST( gtk_object_get_data(
        GTK_OBJECT( p_intf->p_sys->p_playwin ), "playlist_clist" ) );

    /* update the display */
    GtkRebuildCList( p_playlist_clist, p_playlist );

    vlc_object_release( p_playlist );
}


void GtkDiscOpenCancel( GtkButton * button, gpointer user_data )
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


/*****************************************************************************
 * Network stream callbacks
 *****************************************************************************
 * The following callbacks are related to the network stream manager.
 *****************************************************************************/
gboolean GtkNetworkOpenShow( GtkWidget       *widget,
                             gpointer         user_data )
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(widget), (char*)user_data );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_network ) )
    {
        char *psz_channel_server;

        p_intf->p_sys->p_network = create_intf_network();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_network ),
                             "p_intf", p_intf );

        gtk_spin_button_set_value( GTK_SPIN_BUTTON( gtk_object_get_data(
            GTK_OBJECT( p_intf->p_sys->p_network ), "network_udp_port" ) ),
            config_GetInt( p_intf, "server-port" ) );

        psz_channel_server = config_GetPsz( p_intf, "channel-server" );
        if( psz_channel_server )
            gtk_entry_set_text( GTK_ENTRY( gtk_object_get_data(
                GTK_OBJECT( p_intf->p_sys->p_network ), "network_channel_address" ) ),
                psz_channel_server );
        if( psz_channel_server ) free( psz_channel_server );

        gtk_spin_button_set_value( GTK_SPIN_BUTTON( gtk_object_get_data(
            GTK_OBJECT( p_intf->p_sys->p_network ), "network_channel_port" ) ),
            config_GetInt( p_intf, "channel-port" ) );

        gtk_toggle_button_set_active( gtk_object_get_data( GTK_OBJECT(
            p_intf->p_sys->p_network ), "network_channel" ),
            config_GetInt( p_intf, "network-channel" ) );
    }

    gtk_widget_show( p_intf->p_sys->p_network );
    gdk_window_raise( p_intf->p_sys->p_network->window );

    return TRUE;
}


void GtkNetworkOpenOk( GtkButton *button, gpointer user_data )
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_network" );
    playlist_t *    p_playlist;
    GtkCList *      p_playlist_clist;
    char *          psz_source, *psz_address;
    unsigned int    i_port;
    vlc_bool_t      b_channel;

    p_playlist = vlc_object_find( p_intf, VLC_OBJECT_PLAYLIST, FIND_ANYWHERE );
    if( p_playlist == NULL )
    {
        return;
    }

    gtk_widget_hide( p_intf->p_sys->p_network );
//    psz_server = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
//                                 GTK_WIDGET(button), "network_server" ) ) );

    /* select added item */
#if 0
    if( p_intf->p_vlc->p_input_bank->pp_input[0] != NULL )
    {
        p_intf->p_vlc->p_input_bank->pp_input[0]->b_eof = 1;
    }
#endif

    /* Manage channel server */
    b_channel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(
            lookup_widget( GTK_WIDGET(button), "network_channel" ) ) );
    config_PutInt( p_intf, "network-channel", b_channel );

    /* Check which option was chosen */
    /* UDP */
    if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                          "network_udp" ) )->active )
    {
        /* No address in UDP mode */
        psz_address = "";

        /* Get the port number and make sure it will not
         * overflow 5 characters */
        i_port = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(
             lookup_widget( GTK_WIDGET(button), "network_udp_port" ) ) );
        if( i_port > 65535 )
        {
            msg_Err( p_intf, "invalid port %i", i_port );
        }
 
        /* Allocate room for "protocol:@:port" */
        psz_source = malloc( 5 /* "udp:@" */ + 1 /* ":" */
                             + 5 /* 0-65535 */ + 1 /* "\0" */ );
        if( psz_source == NULL )
        {
            return;
        }

        /* Build source name and add it to playlist */
        sprintf( psz_source, "udp:@:%i", i_port );
        playlist_Add( p_playlist, psz_source,
                      PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
        free( psz_source );

        /* catch the GTK CList */
        p_playlist_clist = GTK_CLIST( gtk_object_get_data(
            GTK_OBJECT( p_intf->p_sys->p_playwin ), "playlist_clist" ) );
        /* update the display */
        GtkRebuildCList( p_playlist_clist, p_playlist );
    }

    /* UDP Multicast */
    else if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                               "network_multicast" ) )->active )
    {
        /* get the address */
        psz_address = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                        GTK_WIDGET(button), "network_multicast_address" ) ) );
 
        /* Get the port number and make sure it will not
         * overflow 5 characters */
        i_port = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(
             lookup_widget( GTK_WIDGET(button), "network_multicast_port" ) ) );
        if( i_port > 65535 )
        {
            msg_Err( p_intf, "invalid port %i", i_port );
        }
 
        /* Allocate room for "protocol:@address:port" */
        psz_source = malloc( 5 /* "udp:@" */
                             + strlen( psz_address ) + 1 /* ":" */
                             + 5 /* 0-65535 */ + 1 /* "\0" */ );
        if( psz_source == NULL )
        {
            return;
        }

        /* Build source name and add it to playlist */
        sprintf( psz_source, "udp:@%s:%i", psz_address, i_port );
        playlist_Add( p_playlist, psz_source,
                      PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
        free( psz_source );

        /* catch the GTK CList */
        p_playlist_clist = GTK_CLIST( gtk_object_get_data(
            GTK_OBJECT( p_intf->p_sys->p_playwin ), "playlist_clist" ) );
        /* update the display */
        GtkRebuildCList( p_playlist_clist, p_playlist );
    }
    
    /* Channel server */
    else if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                               "network_channel" ) )->active )
    {
        char *          psz_channel;
        unsigned int    i_channel_port;

        if( p_intf->p_vlc->p_channel == NULL )
        {
            network_ChannelCreate( p_intf );
        }

        psz_channel = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                        GTK_WIDGET(button), "network_channel_address" ) ) );
        i_channel_port = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(
            lookup_widget( GTK_WIDGET(button), "network_channel_port" ) ) );

        config_PutPsz( p_intf, "channel-server", psz_channel );
        if( i_channel_port < 65536 )
        {
            config_PutInt( p_intf, "channel-port", i_channel_port );
        }

        p_intf->p_sys->b_playing = 1;
    }
    
    /* HTTP */
    else if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET(button),
                                               "network_http" ) )->active )
    {
        /* get the url */
        psz_address = gtk_entry_get_text( GTK_ENTRY( lookup_widget(
                        GTK_WIDGET(button), "network_http_url" ) ) );

        /* Allocate room for "protocol://url" */
        psz_source = malloc( 7 /* "http://" */
                             + strlen( psz_address ) + 1 /* "\0" */ );
        if( psz_source == NULL )
        {
            return;
        }

        /* Build source name and add it to playlist */
        sprintf( psz_source, "http://%s", psz_address );
        playlist_Add( p_playlist, psz_source,
                      PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
        free( psz_source );

        /* catch the GTK CList */
        p_playlist_clist = GTK_CLIST( gtk_object_get_data(
            GTK_OBJECT( p_intf->p_sys->p_playwin ), "playlist_clist" ) );
        /* update the display */
        GtkRebuildCList( p_playlist_clist, p_playlist );
    }

    /* This shouldn't occur */
    else
    {
        msg_Err( p_intf, "unknown protocol toggle button position" );
        return;
    }

    /* add the item to the playlist if the channel server wasn't chosen */
    if( !b_channel )
    {
    }

    vlc_object_release( p_playlist );
}

void GtkNetworkOpenCancel( GtkButton * button, gpointer user_data)
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}


void GtkNetworkOpenUDP( GtkToggleButton *togglebutton,
                                        gpointer user_data )
{
    GtkWidget *     p_network;

    p_network = gtk_widget_get_toplevel( GTK_WIDGET (togglebutton) );

    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_udp_port_label" ),
                    gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_udp_port" ),
                    gtk_toggle_button_get_active( togglebutton ) );
}


void GtkNetworkOpenMulticast( GtkToggleButton *togglebutton,
                                              gpointer user_data )
{
    GtkWidget *     p_network;

    p_network = gtk_widget_get_toplevel( GTK_WIDGET (togglebutton) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_multicast_address_label" ),
                    gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_multicast_address_combo" ),
                    gtk_toggle_button_get_active( togglebutton ) );

    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_multicast_port_label" ),
                    gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_multicast_port" ),
                    gtk_toggle_button_get_active( togglebutton ) );
}


void GtkNetworkOpenChannel( GtkToggleButton *togglebutton,
                                       gpointer user_data )
{
    GtkWidget *     p_network;

    p_network = gtk_widget_get_toplevel( GTK_WIDGET (togglebutton) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_channel_address_label" ),
                    gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_channel_address_combo" ),
                    gtk_toggle_button_get_active( togglebutton ) );

    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_channel_port_label" ),
                    gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_channel_port" ),
                    gtk_toggle_button_get_active( togglebutton ) );
}


void GtkNetworkOpenHTTP( GtkToggleButton *togglebutton,
                                         gpointer user_data )
{   
    GtkWidget *     p_network;

    p_network = gtk_widget_get_toplevel( GTK_WIDGET (togglebutton) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_http_url_label" ),
                    gtk_toggle_button_get_active( togglebutton ) );
    gtk_widget_set_sensitive( gtk_object_get_data( GTK_OBJECT( p_network ),
                    "network_http_url" ),
                    gtk_toggle_button_get_active( togglebutton ) );
}


/*****************************************************************************
 * Open satellite callbacks
 *****************************************************************************
 * The following callbacks are related to the satellite card manager.
 *****************************************************************************/
gboolean GtkSatOpenShow( GtkWidget       *widget,
                         gpointer         user_data)
{
    intf_thread_t *p_intf = GetIntf( GTK_WIDGET(widget), (char*)user_data );

    if( !GTK_IS_WIDGET( p_intf->p_sys->p_sat ) )
    {
        p_intf->p_sys->p_sat = create_intf_sat();
        gtk_object_set_data( GTK_OBJECT( p_intf->p_sys->p_sat ),
                             "p_intf", p_intf );
    }

    gtk_widget_show( p_intf->p_sys->p_sat );
    gdk_window_raise( p_intf->p_sys->p_sat->window );

    return TRUE;
}

void GtkSatOpenOk( GtkButton * button, gpointer user_data )
{
    intf_thread_t * p_intf = GetIntf( GTK_WIDGET(button), "intf_sat" );
    playlist_t *    p_playlist;
    GtkCList *      p_playlist_clist;
    char *          psz_source;
    int             i_freq, i_srate;
    int             i_fec;
    vlc_bool_t      b_pol;

    p_playlist = vlc_object_find( p_intf, VLC_OBJECT_PLAYLIST, FIND_ANYWHERE );
    if( p_playlist == NULL )
    {
        return;
    }

    gtk_widget_hide( p_intf->p_sys->p_sat );

    /* Check which polarization was activated */
    if( GTK_TOGGLE_BUTTON( lookup_widget( GTK_WIDGET( button ),
                                        "sat_pol_vert" ) )->active )
    {
        b_pol = 0;
    }
    else
    {
        b_pol = 1;
    }

    i_fec = strtol( gtk_entry_get_text( GTK_ENTRY( GTK_COMBO( 
                lookup_widget( GTK_WIDGET( button ), "sat_fec" )
                )->entry ) ), NULL, 10 );

    /* Select frequency and symbol rate */
    i_freq = gtk_spin_button_get_value_as_int(
                              GTK_SPIN_BUTTON( lookup_widget(
                                  GTK_WIDGET(button), "sat_freq" ) ) );

    i_srate = gtk_spin_button_get_value_as_int(
                              GTK_SPIN_BUTTON( lookup_widget(
                                  GTK_WIDGET(button), "sat_srate" ) ) );
    
    psz_source = malloc( 22 );
    if( psz_source == NULL )
    {
        return;
    }

    /* Build source name and add it to playlist */
    sprintf( psz_source, "%s:%d,%d,%d,%d",
             "satellite", i_freq, b_pol, i_fec, i_srate );
    playlist_Add( p_playlist, psz_source,
                  PLAYLIST_APPEND | PLAYLIST_GO, PLAYLIST_END );
    free( psz_source );

    /* catch the GTK CList */
    p_playlist_clist = GTK_CLIST( gtk_object_get_data(
        GTK_OBJECT( p_intf->p_sys->p_playwin ), "playlist_clist" ) );

    /* update the display */
    GtkRebuildCList( p_playlist_clist, p_playlist );

    vlc_object_release( p_playlist );
}


void GtkSatOpenCancel( GtkButton * button, gpointer user_data )
{
    gtk_widget_hide( gtk_widget_get_toplevel( GTK_WIDGET (button) ) );
}

