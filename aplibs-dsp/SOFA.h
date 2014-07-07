/*
 * SOFA.h
 *
 *  Created on: Jun 18, 2014
 *      Author: chrisp
 */

#ifndef SOFA_H_
#define SOFA_H_

#include <vector>
#include <string>
#include <netcdf>

#include <aplibs-dsp/misc.h>
#include <aplibs-dsp/3DPosition.h>

BBC_AUDIOTOOLBOX_START

class SOFA {
public:
  SOFA(const std::string filename);
  virtual ~SOFA();


  typedef netCDF::NcFile                          sofa_file_t;
  typedef netCDF::NcVar                           sofa_var_t;
  typedef netCDF::NcDim                           sofa_dim_t;
  typedef netCDF::NcGroupAtt                      sofa_att_t;
  typedef netCDF::NcVarAtt                        sofa_varatt_t;
  typedef std::multimap<std::string,sofa_var_t>   sofa_var_collection_t;
  typedef std::multimap<std::string,sofa_dim_t>   sofa_dim_collection_t;
  typedef std::vector<sofa_dim_t>                 sofa_dims_vec_t;
  typedef std::multimap<std::string,sofa_att_t>   sofa_att_collection_t;
  typedef std::map<std::string,sofa_varatt_t>     sofa_varatt_collection_t;
  typedef std::vector<size_t>                     get_index_vec_t;
  typedef float                                   audio_sample_t;
  typedef std::vector<audio_sample_t>             audio_buffer_t;
  typedef float                                   delay_sample_t;
  typedef std::vector<audio_sample_t>             delay_buffer_t;
  typedef std::vector<bbcat::Position>            positions_array_t;

  operator bool () const { return (sofa_file != nullptr); }

  std::string         get_convention_name() const;
  float               get_samplerate() const;
  bool                get_ir(audio_buffer_t& ir_buffer, uint_t indexM, uint_t indexR, uint_t indexE = 0) const;
  bool                get_ir(float* ir_buffer, uint_t indexM, uint_t indexR, uint_t indexE = 0) const;
  bool                get_delays(delay_buffer_t& delays, uint_t indexR, uint_t indexE = 0) const;
  size_t              get_num_measurements() const;
  size_t              get_ir_length() const;
  size_t              get_num_receivers() const;
  size_t              get_num_emitters() const;
  positions_array_t   get_source_positions() const;
  positions_array_t   get_emitter_positions() const;
  positions_array_t   get_listener_positions() const;
  positions_array_t   get_receiver_positions() const;
  positions_array_t   get_listener_view_vecs() const;
  positions_array_t   get_listener_up_vecs() const;
  void                list_vars() const;
  void                list_atts() const;

protected:
  void list_varatts(const sofa_var_t sofa_var) const;
  sofa_var_t get_var(const std::string var_name) const;
  sofa_att_t get_att(const std::string att_name) const;
  bool get_ir_data(get_index_vec_t start, get_index_vec_t count, audio_buffer_t& ir_buffer) const;
  bool get_ir_data(get_index_vec_t start, get_index_vec_t count, float* ir_buffer) const;
  bool get_delay_data(get_index_vec_t start, get_index_vec_t count, delay_buffer_t& delays) const;
  positions_array_t get_position_var_data(const std::string position_variable_name) const;
private:
  struct {
    netCDF::NcDim N,M,R,E,C,I;
  } sofa_dims;
  sofa_file_t* sofa_file;
  std::string convention_name;
  float sample_rate;
};

BBC_AUDIOTOOLBOX_END

#endif /* SOFA_H_ */
