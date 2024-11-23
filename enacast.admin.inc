<?php

/**
 * @file
 * Administrative page callbacks for the Enacast module.
 */


/**
 * Form builder for Enacast settings.
 *
 * @param array $form
 *   An associative array containing the initial structure of the form.
 * @param array &$form_state
 *   An associative array containing the current state of the form.
 *
 * @return array
 *   The form structure.
 */
function enacast_settings_form($form, &$form_state) {
  $form['#config'] = 'enacast.settings';

  $config = config('enacast.settings');

  $form['type_of_site'] = array(
    '#type' => 'radios',
    '#title' => t('Type of site'),
    '#default_value' => $config->get('type_of_site'),
    '#options' => array(
      'radio' => t('Radio'),
      'program' => t('Program'),
    ),
    '#description' => t('Specify if this is a Radio station website (nodes for programmes and podcasts) or a Program website (nodes for podcasts).'),
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
    $program_instances = field_info_instances('node', $config->get('program_type'));
    $has_program_field = FALSE;
    foreach ($program_instances as $key => $instance) {
      $field = field_info_field($instance['field_name']);
      if ($field['type'] == 'enacast_program') {
        $has_program_field = TRUE;
        $config->set('program_field', $field['field_name']);
        $form['program_field'] = array(
          '#type' => 'textfield',
          '#title' => t('Program field'),
          '#default_value' => $config->get('program_field'),
          '#disabled' => TRUE,
        );
        break;
      }
    }
    if (!$has_program_field) {
      backdrop_set_message(
        t('Content type "@type" needs to have an "Enacast program" field.', array('@type' => $types[$config->get('program_type')]->name)),
        'error'
      );
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
    $podcast_instances = field_info_instances('node', $config->get('podcast_type'));
    $has_podcast_field = FALSE;

    foreach ($podcast_instances as $key => $instance) {
      $field = field_info_field($instance['field_name']);
      if ($field['type'] == 'enacast_podcast') {
        $has_podcast_field = TRUE;
        $config->set('podcast_field', $field['field_name']);
        $form['podcast_field'] = array(
          '#type' => 'textfield',
          '#title' => t('Podcast field'),
          '#default_value' => $config->get('podcast_field'),
          '#disabled' => TRUE,
        );
        break;
      }
    }

    if (!$has_podcast_field) {
      backdrop_set_message(
        t('Content type "@type" needs to have an "Enacast podcast PK ID" field.', array('@type' => $types[$config->get('podcast_type')]->name)),
        'error'
      );
    }
  }

  return system_settings_form($form);
}

/**
 * Callback page for updating Enacast data into local tables.
 *
 * @return array
 *   A renderable array representing the update content page.
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
        $row[] = l(t('View Node'), 'node/' . $nid);
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
    $header = array(
      'pkid',
      'name',
      'node'
    );
    $build['ajax_table_wrapper'] = array(
      '#prefix' => '<div id="ajax-table-wrapper">',
      '#suffix' => '</div>',
    );
    $build['ajax_table_wrapper']['table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#caption' => t('Podcasts in Enacast local tables'),
      '#ajax' => array(
        'wrapper' => 'ajax-table-wrapper',
        'effect' => 'fade',
      ),
      '#attributes' => array(
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
  // $form['#config'] = 'enacast.settings';
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
    if (empty($config->get('radio')) || empty($config->get('program'))) {
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
        '#attributes' => array('class' => ['button-primary']),
        '#submit' => array('enacast_update_form_submit'),
      );
      $form['update']['check'] = array(
        '#type' => 'submit',
        '#value' => t('Check json data'),
        '#attributes' => array('class' => ['button']),
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

    if (!empty($data['results'])) {
      foreach ($data['results'] as $podcast_json) {
        $podcast_db = enacast_podcast_load($podcast_json['id']);
        if (!$podcast_db) {
          enacast_podcast_insert($podcast_json);
          if (!empty($podcast_json['markers'])) {
            foreach ($podcast_json['markers'] as $marker) {
              enacast_marker_insert($podcast_json, $marker);
            }
          }
        }
        else {
          backdrop_set_message(
            t('Cannot insert already existing podcast @podcast.', array('@podcast' => $podcast_db['pkid'])),
            'error'
          );
        }
      }
    }
    else {
      backdrop_set_message(t('No new podcasts found to import.'), 'status');
    }
  }

  $form_state['redirect'] = array(
    'admin/config/services/enacast/update',
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
 * Loads Enacast podcast data for a given podcast ID if it exists.
 *
 * @param string $id
 *   The podcast ID.
 *
 * @return array|bool
 *   An associative array of podcast data or FALSE if not found.
 */
function enacast_podcast_load($id) {
  $fields = array(
    'id',
    'pkid',
    'name',
    'summary',
    'audio',
    'program_codename',
    'program_name',
    'radio_codename',
    'radio_name',
    'utc_datetime',
    'datetime_tz',
  );
  $result = db_select('enacast_podcasts', 'p')
    ->fields('p', $fields)
    ->condition('id', $id)
    ->execute()
    ->fetchAssoc();

  return $result ?: FALSE;
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
    'id',
    'pkid',
    'name',
    'summary',
    'audio',
    'program_codename',
    'program_name',
    'radio_codename',
    'radio_name',
    'utc_datetime',
    'datetime_tz',
  );
  $podcast['pkid'] = enacast_get_pkid($podcast);
  $insert_data = array_intersect_key($podcast, array_flip($fields));

  try {
    return db_insert('enacast_podcasts')
      ->fields($insert_data)
      ->execute();
  }
  catch (Exception $e) {
    backdrop_set_message(t('Database insert failed: @message', array('@message' => $e->getMessage())), 'error');
    return NULL;
  }
}

/**
 * Inserts marker data for a given podcast into the Enacast local tables.
 *
 * @param array $podcast
 *   The podcast data array.
 * @param array $marker
 *   The marker data array.
 *
 * @return int|null
 *   The insert ID or NULL on failure.
 */
function enacast_marker_insert($podcast, $marker) {
  $fields = array(
    'pid' => $podcast['id'],
    'pkid' => enacast_get_pkid($podcast),
    'mid' => $marker['id'],
    'descr' => $marker['descr'],
    'startsecond' => $marker['startsecond'],
  );

  try {
    return db_insert('enacast_markers')
      ->fields($fields)
      ->execute();
  }
  catch (Exception $e) {
    backdrop_set_message(t('Database insert failed: @message', array('@message' => $e->getMessage())), 'error');
    return NULL;
  }
}

/**
 * Retrieves a paged set of all Enacast podcasts from local tables.
 *
 * @return array
 *   An array of podcast objects.
 */
function enacast_podcasts_get_all() {
  $fields = array(
    'id',
    'pkid',
    'name',
    'summary',
    'audio',
    'program_codename',
    'program_name',
    'radio_codename',
    'radio_name',
    'utc_datetime',
    'datetime_tz',
  );
  $result = db_select('enacast_podcasts', 'p')
    ->extend('PagerDefault')
    ->fields('p', $fields)
    ->limit(10)
    ->orderBy('p.datetime_tz', 'DESC')
    ->execute()
    ->fetchAll();

  return $result;
}

/**
 * Returns a podcast PK ID extracted from the audio URL.
 *
 * @param array $podcast
 *   The podcast data array.
 *
 * @return string
 *   The extracted PK ID.
 */
function enacast_get_pkid($podcast) {
  if (preg_match('/([\d\.]+)\.mp3$/', $podcast['audio'], $matches)) {
    return $matches[1];
  }
  return '';
}

/**
 * Gets the corresponding node ID of a local podcast.
 *
 * @param object $podcast
 *   The podcast object.
 *
 * @return int|bool
 *   The node ID if found, FALSE otherwise.
 */
function enacast_get_podcast_node($podcast) {
  $config = config('enacast.settings');
  $podcast_field = $config->get('podcast_field');

  $select = db_select('node', 'n');
  $select->leftJoin('field_data_' . $podcast_field, 'p', 'n.nid=p.entity_id');
  $select->fields('n', array('nid'));
  $select->condition('p.' . $podcast_field . '_value', $podcast->pkid, '=');
  return $select->execute()->fetchField();
}
