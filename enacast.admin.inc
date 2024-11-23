<?php

/**
 * @file
 * Administrative page callbacks for the Enacast module.
 */

//TODO: Document functions

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

  $types = node_type_get_types();
  $options = array(
    'none' => t('None'),
  );
  foreach ($types as $type) {
    $options[$type->type]  = $type->name;
  }
  $form['program_type'] = array(
    '#type' => 'select',
    '#title' => t('Program type'),
    '#default_value' => $config->get('program_type'),
    '#options' => $options,
    '#description' => t('Node type to act as program node.'),
  );
  if ($config->get('program_type') != 'none') {

    // Enacast program field for programmes
    $program_instances = field_info_instances('node', $config->get('program_type'));
    $check = FALSE;
    foreach ($program_instances as $key => $value) {
      $field = field_info_field($value['field_name']);
      if ($field['type'] == 'enacast_program') {
        $check = TRUE;
        $config->set('program_field', $field['field_name']);
        $form['program_field'] = array(
          '#type' => 'textfield',
          '#title' => t('Program field'),
          '#default_value' => $config->get('program_field'),
          '#disabled' => TRUE,
        );
      }
    }
    if ($check === FALSE) {
      backdrop_set_message(t('@type type nodes need to have a "Enacast program" field.', array('@type' => $types[$config->get('program_type')]->name)), 'error');
    }
  }


  $form['podcast_type'] = array(
    '#type' => 'select',
    '#title' => t('Podcast type'),
    '#default_value' => $config->get('podcast_type'),
    '#options' => $options,
    '#description' => t('Node type to act as podcast node.'),
  );
  if ($config->get('podcast_type') != 'none') {

    // Enacast podcast pk id field for podcasts
    $podcast_instances = field_info_instances('node', $config->get('podcast_type'));
    $check = FALSE;
    foreach ($podcast_instances as $key => $value) {
      $field = field_info_field($value['field_name']);
      if ($field['type'] == 'enacast_podcast') {
        $check = TRUE;
        $config->set('podcast_field', $field['field_name']);
        $form['podcast_field'] = array(
          '#type' => 'textfield',
          '#title' => t('Podcast field'),
          '#default_value' => $config->get('podcast_field'),
          '#disabled' => TRUE,
        );
      }
    }
    if ($check === FALSE) {
      backdrop_set_message(t('@type type nodes need to have a "Enacast podcast pk id" field.', array('@type' => $types[$config->get('podcast_type')]->name)), 'error');
    }
  }

  return system_settings_form($form);
}

/**
 * Callback page for updating Enacast data into local tables
 *
 * @return array
 */
function enacast_update_content_page() {
  $config = config('enacast.settings');
  $build = array();

  // printing the form
  $build['form'] = backdrop_get_form('enacast_update_form');

  //printing already imported podcasts
  if ($entries = enacast_podcasts_get_all()) {
    $rows = array();
    foreach ($entries as $entry) {
      $row = array($entry->pkid, $entry->name);
      $nid = enacast_get_podcast_node($entry);
      if ($nid) {
        $row[] = l($nid, 'node/' . $nid);
      } else {
        // TODO: add other node field (date creation as podcast date?...)
        $link_options = array(
          'query' => array(
            'edit[title]' => $entry->name,
            'edit[' . $config->get('podcast_field') . '][und][0][value]' => $entry->pkid,
            'edit[body][und][0][value]' => $entry->summary,  //TODO: What if there is not body?
          ),
        );
        $row[] = l(t('Create node'), '/node/add/' . $config->get('podcast_type'), $link_options);
      }

      $rows[] = $row;
    }
    // Make a table for them.
    $fields = array(
      'pkid', 'name', 'node'
    );
    // Other fields available in the json
    // $fields = array(
    //   'id', 'name', 'summary', 'audio', 'program_codename', 'program_name', 'radio_codename', 'radio_name', 'utc_datetime', 'datetime_tz',
    // );
    $build['ajax_table_wrapper'] = array(
      '#prefix' => '<div id="ajax-table-wrapper">',
      '#suffix' => '</div>',
  );
    $build['ajax_table_wrapper']['table'] = array(
      '#theme' => 'table',
      '#header' => $fields,
      '#rows' => $rows,
      '#caption' => t('Podcasts in Enacast local tables'),
      '#ajax' => array(
        'wrapper' => 'ajax-table-wrapper',
        'effect' => 'fade',
      ),
      '#attributes'=> array(
        'class' => array('enacast-table'),
      ),
    );
    $build['ajax_table_wrapper']['pager'] = array(
      '#markup' => theme('pager'),
    );
  } else {
    backdrop_set_message(t('No entries have been added yet.'));
  }

  return $build;
}

/**
 * Prints the update form for the update page
 *
 * @param array $form
 * @param array $form_state
 * @return array
 *
 * @see enacast_update_content_page()
 */
function enacast_update_form($form, &$form_state) {
  $form['#config'] = 'enacast.settings';
  $config = config('enacast.settings');

  $program = $_GET['program'] ? $_GET['program'] : $config->get('program');
  $page = $_GET['p'] ? $_GET['p'] : 1;


  if ($config->get('type_of_site' == 'program')) {
    $form['update'] = array(
      '#type' => 'fieldset',
      '#title' => t('Import podcasts'),
    );
    $form['update']['program'] = array(
      '#type' => 'textfield',
      '#title' => t('Program'),
      '#value' => $program,
      '#description' => t('Program codename to import podcasts from.'),
    );
    $form['update']['page'] = array(
      '#type' => 'number',
      '#title' => t('page'),
      '#default_value' => $page,
      '#description' => t('Page number for paged json files.'),
    );
    $form['update']['radio'] = array(
      '#type' => 'hidden',
      '#value' => $config->get('radio'),
    );
    $form['update']['import_type'] = array(
      '#type' => 'hidden',
      '#value' => 'podcasts',
    );
    if (($config->get('radio') == '') || ($config->get('program') == '')) {
      backdrop_set_message(t('You need to config this module first on !link', array('!link' => l('Enacast settings', 'admin/config/services/enacast'))), 'error');
      $form['update']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Import new podcast data'),
        '#disabled' => TRUE,
      );
    } else {
      $form['update']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Import new podcast data'),
        '#attributes' => array('class'=>['button-primary']),
        '#submit' => array('enacast_update_form_submit'),
      );
      $form['update']['check'] = array(
        '#type' => 'submit',
        '#value' => t('Check json data'),
        '#attributes' => array('class'=>['button']),
        '#submit' => array('enacast_check_form_submit'),
      );
    }
    $form['update']['markup'] = array(
      '#markup' => '<div>' . t('Use this form to import json data to the Enacast local tables. After that, you will be able to insert data as nodes.') . '</div>'
    );
  }
  // Show the json data if "Check json data" button was cliked
  if (isset($form_state['storage']['debug_info'])) {
    $form['debug'] = array(
        '#markup' => '<pre class="enacast-debug-info">' . $form_state['storage']['debug_info'] . '</pre>',
    );
}
  return $form;
}

/**
 * Submit handler for enacast_update_form()
 *
 * @param array $form
 * @param array $form_state
 * @return void
 *
 * @see enacast_update_form()
 */
function enacast_update_form_submit($form, &$form_state) {
  $program = $form_state['values']['program'];
  $radio = $form_state['values']['radio'];
  $page = $form_state['values']['page'];
  $import_type = $form_state['values']['import_type'];
  if ($import_type == 'podcasts') {
    $url = 'https://enacast.com/api/v3/podcast/?page=' . $page . '&program=' . $program . '&radio=' . $radio;
    $data = enacast_get_json($url);
    foreach ($data['results'] as $podcast_json) {
      $podcast_db = enacast_podcast_load($podcast_json['id']);
      if (!$podcast_db) { //podcast doesn't exist on our database: create it
        enacast_podcast_insert($podcast_json);
        foreach ($podcast_json['markers'] as $marker) {
          enacast_marker_insert($podcast_json, $marker);
        }
      } else { //TODO: podcast does exist on our database: update it?
        backdrop_set_message(t('Cannot insert already existing podcast @podcast', array('@podcast' => $podcast_db['pkid'])), 'error');
      }
    }
  }
  $path = parse_url($form['#action']);
  $path = $path['path'];
  $form_state['redirect'] = array(
    $path,
    array(
      'query' => array(
        'p' => $page,
        'program' => $program,
      ),
    ),
  );
}

/**
 * Submit handler for enacast_check_form()
 *
 * @param array $form
 * @param array $form_state
 * @return void
 *
 * @see enacast_check_form()
 */
function enacast_check_form_submit($form, &$form_state) {
  $program = $form_state['values']['program'];
  $radio = $form_state['values']['radio'];
  $page = $form_state['values']['page'];
  $import_type = $form_state['values']['import_type'];
  if ($import_type == 'podcasts') {
    $url = 'https://enacast.com/api/v3/podcast/?page=' . $page . '&program=' . $program . '&radio=' . $radio;
    $data = enacast_get_json($url);
    $form_state['storage']['debug_info'] = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
  }
  $form_state['rebuild'] = TRUE;

}

/**
 * Returns enacast podcast data for a given podcast_id if it exists.
 *
 * @param string $id
 * @return array
 */
function enacast_podcast_load($id) {
  $fields = array(
    'id', 'pkid', 'name', 'summary', 'audio', 'program_codename', 'program_name', 'radio_codename', 'radio_name', 'utc_datetime', 'datetime_tz',
  );
  $select = db_select('enacast_podcasts', 'p');
  $select->fields('p', $fields);
  $select->condition('id', $id);
  $result = $select->execute()->fetchAssoc();
  return $result;
}

/**
 * Insert a given podcast data into enacast local table.
 * Returns null on error.
 *
 * @param array $podcast
 * @return object|null
 */
function enacast_podcast_insert($podcast) {
  $fields = array(
    'id', 'pkid', 'name', 'summary', 'audio', 'program_codename', 'program_name', 'radio_codename', 'radio_name', 'utc_datetime', 'datetime_tz',
  );
  $insert = array();
  $podcast['pkid'] = enacast_get_pkid($podcast);
  foreach ($fields as $key) {
    $insert[$key] = $podcast[$key];
  }
  $return_value = NULL;
  try {
    $return_value = db_insert('enacast_podcasts')
      ->fields($insert)
      ->execute();
  } catch (Exception $e) {
    backdrop_set_message(t(
      'db_insert failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)
    ), 'error');
  }
  return $return_value;
}

/**
 * Insert a marker data for a given podcast and marker into
 * local enacast tables
 *
 * @param array $podcast
 * @param array $marker
 * @return object|null
 */
function enacast_marker_insert($podcast, $marker) {

  $fields = array(
    'pid' => $podcast['id'],
    'pkid' => enacast_get_pkid($podcast),
    'mid' => $marker['id'],
    'descr' => $marker['descr'],
    'startsecond' => $marker['startsecond'],
  );
  $return_value = NULL;
  try {
    $return_value = db_insert('enacast_markers')
      ->fields($fields)
      ->execute();
  } catch (Exception $e) {
    backdrop_set_message(t(
      'db_insert failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)
    ), 'error');
  }
  return $return_value;
}

/**
 * Gets a paged set of all Enacast podcasts on local tables
 *
 * @return array
 */
function enacast_podcasts_get_all() {
  $fields = array(
    'id', 'pkid', 'name', 'summary', 'audio', 'program_codename', 'program_name', 'radio_codename', 'radio_name', 'utc_datetime', 'datetime_tz',
  );
  $select = db_select('enacast_podcasts', 'p')->extend('PagerDefault');
  $select->fields('p', $fields)
    ->limit(10)
    ->orderBy('p.datetime_tz', 'DESC');
  $result = $select->execute()->fetchAll();
  return $result;
}

/**
 * Returns a podcast pkid
 *
 * @param array $podcast
 * @return string
 */
function enacast_get_pkid($podcast) {
  // dpm($podcast);
  preg_match('/([.\d]*).mp3/', $podcast['audio'], $matches);
  return $matches[1];
}

/**
 * Gets the corresponding node of a local podcast
 *
 * @param object $podcast
 * @return void
 */
function enacast_get_podcast_node($podcast) {
  // gets the corresponding node of a given podcast
  $config = config('enacast.settings');
  // $podcast_type = $config->get('podcast_type');
  // dpm($config);
  $select = db_select('node', 'n');
  $select->leftJoin('field_data_' . $config->get('podcast_field'), 'p', 'n.nid=p.entity_id');
  $select->fields('n', array('nid'));
  $select->condition('p.' . $config->get('podcast_field') . '_value', $podcast->pkid, '=');
  return $select->execute()->fetchField();
}
