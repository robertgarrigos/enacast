<?php
/**
 * @file
 * Administrative page callbacks for the Enacast module.
 */

/**
 * Enacast configuration form.
 */
function enacast_settings_form($form, &$form_state) {
  $form['#config'] = 'enacast.settings';

  $config = config('enacast.settings');

  $form['type_of_site'] = array(
    '#type' => 'radios',
    '#title' => t('Type of site'),
    '#default_value' => $config->get('type_of_site'),
    '#options' => array(
      'radio' => t('radio'),
      'program' => t('program'),
    ),
    '#description' => t('Whether this is a Radio station website (nodes for programmes and podcasts) or a Program website (nodes for podcasts).'),
  );

  $form['radio'] = array(
    '#type' => 'textfield',
    '#title' => t('Radio'),
    '#default_value' => $config->get('radio'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => TRUE,
    '#description' => t('Machine name for the radio station.')
  );

  $form['program'] = array(
    '#type' => 'textfield',
    '#title' => t('Program'),
    '#default_value' => $config->get('program'),
    '#size' => 60,
    '#maxlength' => 128,
    '#required' => FALSE,
    '#description' => t('Machine name for the program. Only used if type of website is Program.')
  );

  return system_settings_form($form);
}
