/**********************************************************************************************/
/* config.h				  		                                                   			  */
/*                                                                       					  */
/* Igor Nikitin, 2013																		  */
/* MIT license			                                                  					  */
/**********************************************************************************************/

#pragma once

/**********************************************************************************************/
#define the_config config::instance()


/**********************************************************************************************/
class config
{
	public://////////////////////////////////////////////////////////////////////////

// This class API:		
		
	// ---------------------
	// Properties

							/** Returns kind of authorization for each request. */
		cr_http_auth		get_auth_kind( void ) const;
		const char*			set_auth_kind( const char* value );
		
							/** Returns default format which will used if HTTP header
							 *  'Accept' doesn't contain nor XML nor JSON format. */
		cr_result_format	get_default_format( void ) const;
		const char*			set_default_format( const char* value );
		
							/** Returns TRUE if each request will be log into
							 *  file on disk. */
		bool				get_log_enabled( void ) const;
		const char*			set_log_enabled( const char* value );
		
							/** Returns ports which listen SLITE, ports separate be comma,
							 *  SSL ports have 's' suffix. */
		string				get_ports( void ) const;
		const char*			set_ports( const string& ports );
		
							/** Returns count of thread to process requests. */
		size_t				get_thread_count( void ) const;
		const char*			set_thread_count( const char* value );
		
		
	public://////////////////////////////////////////////////////////////////////////

	// ---------------------
	// Methods		
		
							/** Returns singleton. */
static	config&				instance( void );

							/** Sets destination file and load entries from it if exists. */
		void				set_file( const string& file );


	public://////////////////////////////////////////////////////////////////////////
		
	// ---------------------
	// Save/load	
		
		void				flush( void );
		void				load( void );

		
	protected://////////////////////////////////////////////////////////////////////////
		
// Properties		
		
		cr_http_auth		auth_				= CR_AUTH_NONE;
		string				config_file_;
		cr_result_format	default_format_		= CR_FORMAT_JSON;
		bool				log_enabled_		= true;
		string				ports_				= "15555";
		size_t				thread_count_		= 8;

		
// Concurency
		
mutable	mutex				mutex_;	
};
